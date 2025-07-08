/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sm_manager.h"

#include <sys/stat.h>
#include <unistd.h>

#include <fstream>

#include "index/ix.h"
#include "record/rm.h"
#include "record_printer.h"
#include "record/rm_manager.h"

/**
 * @description: 判断是否为一个文件夹
 * @return {bool} 返回是否为一个文件夹
 * @param {string&} db_name 数据库文件名称，与文件夹同名
 */
bool SmManager::is_dir(const std::string& db_name) {
    struct stat st;
    return stat(db_name.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

/**
 * @description: 创建数据库，所有的数据库相关文件都放在数据库同名文件夹下
 * @param {string&} db_name 数据库名称
 */
void SmManager::create_db(const std::string& db_name) {
    if (is_dir(db_name)) {
        throw DatabaseExistsError(db_name);
    }
    //为数据库创建一个子目录
    std::string cmd = "mkdir " + db_name;
    if (system(cmd.c_str()) < 0) {  // 创建一个名为db_name的目录
        throw UnixError();
    }
    if (chdir(db_name.c_str()) < 0) {  // 进入名为db_name的目录
        throw UnixError();
    }
    //创建系统目录
    DbMeta *new_db = new DbMeta();
    new_db->name_ = db_name;

    // 注意，此处ofstream会在当前目录创建(如果没有此文件先创建)和打开一个名为DB_META_NAME的文件
    std::ofstream ofs(DB_META_NAME);

    // 将new_db中的信息，按照定义好的operator<<操作符，写入到ofs打开的DB_META_NAME文件中
    ofs << *new_db;  // 注意：此处重载了操作符<<

    delete new_db;

    // 创建日志文件
    disk_manager_->create_file(LOG_FILE_NAME);

    // 回到根目录
    if (chdir("..") < 0) {
        throw UnixError();
    }
}

/**
 * @description: 删除数据库，同时需要清空相关文件以及数据库同名文件夹
 * @param {string&} db_name 数据库名称，与文件夹同名
 */
void SmManager::drop_db(const std::string& db_name) {
    if (!is_dir(db_name)) {
        throw DatabaseNotFoundError(db_name);
    }
    std::string cmd = "rm -r " + db_name;
    if (system(cmd.c_str()) < 0) {
        throw UnixError();
    }
}

/**
 * @description: 打开数据库，找到数据库对应的文件夹，并加载数据库元数据和相关文件
 * @param {string&} db_name 数据库名称，与文件夹同名
 */
void SmManager::open_db(const std::string& db_name) {
    if (!is_dir(db_name))
        throw DatabaseNotFoundError(db_name);
    if (chdir(db_name.c_str()) < 0)   // 改变当前线程的工作路进到db_name
        throw UnixError();
    std::ifstream ifs(DB_META_NAME); // 打开数据库元数据文件
    ifs >> db_; // 读取数据库元数据

    // 进行其他打开数据库的操作，例如加载表信息、初始化缓存等
    // 创建fhs,ihs
    for (auto &entry: db_.tabs_) { // 遍历tabs
        const std::string &tab_name = entry.first;
        fhs_.emplace(tab_name, rm_manager_->open_file(tab_name));
        for(auto &ix: entry.second.indexes) { // TabMeta中的IndexMeta
            ihs_.emplace(ix_manager_->get_index_name(tab_name, ix.cols),
                         ix_manager_->open_index(tab_name, ix.cols));
        }
    }
}

/**
 * @description: 把数据库相关的元数据刷入磁盘中
 */
void SmManager::flush_meta() {
    // 默认清空文件
    std::ofstream ofs(DB_META_NAME);
    ofs << db_;
}

/**
 * @description: 关闭数据库并把数据落盘
 */
void SmManager::close_db() {
    flush_meta();

    for (auto &entry : fhs_) {
        auto &fh = entry.second;
        rm_manager_->close_file(fh.get()); // 记录文件落盘
    }
    for(auto &entry : ihs_) {
        auto &ih = entry.second;
        ix_manager_->close_index(ih.get()); // 索引文件落盘
    }

    fhs_.clear();
    ihs_.clear();

    if(chdir("..") < 0) throw UnixError();
}

/**
 * @description: 显示所有的表,通过测试需要将其结果写入到output.txt,详情看题目文档
 * @param {Context*} context 
 */
void SmManager::show_tables(Context* context) {
    std::fstream outfile;
    outfile.open("output.txt", std::ios::out | std::ios::app);
    outfile << "| Tables |\n";
    RecordPrinter printer(1);
    printer.print_separator(context);
    printer.print_record({"Tables"}, context);
    printer.print_separator(context);
    for (auto &entry : db_.tabs_) {
        auto &tab = entry.second;
        printer.print_record({tab.name}, context);
        outfile << "| " << tab.name << " |\n";    }
    printer.print_separator(context);
    outfile.close();
}

/**
 * @description: 显示表的元数据
 * @param {string&} tab_name 表名称
 * @param {Context*} context 
 */
void SmManager::desc_table(const std::string& tab_name, Context* context) {
    TabMeta &tab = db_.get_table(tab_name);

    std::vector<std::string> captions = {"Field", "Type", "Index"};
    RecordPrinter printer(captions.size());
    // Print header
    printer.print_separator(context);
    printer.print_record(captions, context);
    printer.print_separator(context);
    // Print fields
    for (auto &col : tab.cols) {
        std::vector<std::string> field_info = {col.name, coltype2str(col.type), col.index ? "YES" : "NO"};
        printer.print_record(field_info, context);    }
    // Print footer
    printer.print_separator(context);
}

/**
 * @description: 创建表
 * @param {string&} tab_name 表的名称
 * @param {vector<ColDef>&} col_defs 表的字段
 * @param {Context*} context 
 */
void SmManager::create_table(const std::string& tab_name, const std::vector<ColDef>& col_defs, Context* context) {
    if (db_.is_table(tab_name)) {
        throw TableExistsError(tab_name);
    }
    // Create table meta
    int curr_offset = 0;
    TabMeta tab;
    tab.name = tab_name;
    for (auto &col_def : col_defs) {
        ColMeta col = {.tab_name = tab_name,
                .name = col_def.name,
                .type = col_def.type,
                .len = col_def.len,
                .offset = curr_offset,
                .index = false};
        curr_offset += col_def.len;
        tab.cols.push_back(col);
    }
    // Create & open record file
    int record_size = curr_offset;  // record_size就是col meta所占的大小（表的元数据也是以记录的形式进行存储的）
    rm_manager_->create_file(tab_name, record_size);
    db_.tabs_[tab_name] = tab;
    // fhs_[tab_name] = rm_manager_->open_file(tab_name);
    fhs_.emplace(tab_name, rm_manager_->open_file(tab_name));

    flush_meta();
}

/**
 * @description: 删除表
 * @param {Context*} context
 */
void SmManager::drop_table(const std::string& tab_name, Context* context) {
    if(!db_.is_table(tab_name))
        throw TableNotFoundError(tab_name);

    // 删除索引
    auto indexes = db_.get_table(tab_name).indexes;
    for(auto &index: indexes)
        drop_index(tab_name, index.cols, context);

    // 删除表
    auto &fh = fhs_[tab_name];
    fh->clear_pages(); 
    rm_manager_->close_file(fh.get());   // 关闭文件
    rm_manager_->destroy_file(tab_name); // 删除文件

    // 元数据清理
    db_.tabs_.erase(tab_name);
    fhs_.erase(tab_name);
    flush_meta();
}

/**
 * @description: 创建索引
 * @param {string&} tab_name 表的名称
 * @param {vector<string>&} col_names 索引包含的字段名称
 * @param {Context*} context
 */
void SmManager::create_index(const std::string& tab_name, const std::vector<std::string>& col_names, Context* context) {
    // 创建索引对象
    std::vector<ColMeta> cols;
    int col_tot_len = 0;
    for(size_t i = 0; i < col_names.size(); ++i) {
        auto col_meta = db_.get_table(tab_name).get_col(col_names[i]);
        col_tot_len += col_meta->len;
        cols.push_back(*col_meta);
    }
    
    ix_manager_->create_index(tab_name, cols);
    auto &&ih = ix_manager_->open_index(tab_name, cols);
    
    // 遍历表中的所有记录，并将其插入到索引中
    RmFileHandle *fh = fhs_[tab_name].get();
    RmScan scan(fh);

    for(; !scan.is_end(); scan.next()) {
        // 获取构建key所需的数据
        Rid rid = scan.rid();
        std::unique_ptr<char> key(new char[col_tot_len]);
        auto record = fh->get_record(rid, context);
        // 将记录中的索引字段数据拷贝到key中
        int offset = 0;
        for(const ColMeta &col : cols) {
            memcpy(key.get() + offset, record->data + col.offset, col.len);
            offset += col.len;
        } 
        if(!ih->is_unique(key.get(), context->txn_)) { // 注意判断唯一性
            // 注意将索引删掉
            ix_manager_->close_index(ih.get());
            ix_manager_->destroy_index(tab_name, cols);
            throw IndexNotUniqueError(ix_manager_->get_index_name(tab_name, col_names)); 
        }

        // 更新表中的索引元数据，放在最后是为了避免因为索引创建失败但有索引元数据而导致的错误
   
        ih->insert_entry(key.get(), rid, context->txn_);   
    }

    db_.get_table(tab_name).indexes. \
        push_back(IndexMeta{tab_name, col_tot_len, (int)cols.size(), cols});
    ihs_.emplace(ix_manager_->get_index_name(tab_name, cols), std::move(ih));

    flush_meta();
}

/**
 * @description: 删除索引
 * @param {string&} tab_name 表名称
 * @param {vector<string>&} col_names 索引包含的字段名称
 * @param {Context*} context
 */
void SmManager::drop_index(const std::string& tab_name, const std::vector<std::string>& col_names, Context* context) {
    // 基本是create_index的逆过程
    if(!db_.get_table(tab_name).is_index(col_names))
        throw IndexNotFoundError(tab_name, col_names);

    // 删除索引元数据
    std::string index_name = ix_manager_->get_index_name(tab_name, col_names);
    auto index_meta = db_.get_table(tab_name).get_index_meta(col_names);
    ix_manager_->close_index(ihs_.at(index_name).get());
    ix_manager_->destroy_index(tab_name, index_meta->cols);
    db_.get_table(tab_name).indexes.erase(index_meta);
    ihs_[index_name]->clear_pages(); // 清空索引文件中的所有页面
    ihs_.erase(index_name);

    flush_meta();

}

/**
 * @description: 删除索引
 * @param {string&} tab_name 表名称
 * @param {vector<ColMeta>&} 索引包含的字段元数据
 * @param {Context*} context
 */
void SmManager::drop_index(const std::string& tab_name, const std::vector<ColMeta>& cols, Context* context) {
    std::vector<std::string> col_names;
    for(int i=0;i<cols.size();i++)
        col_names.push_back(cols[i].name);
    drop_index(tab_name, col_names, context);
}

void SmManager::show_index(std::string &table_name, Context *context) {
    TabMeta &tab = db_.get_table(table_name);
    if(tab.indexes.empty()) return;

    std::fstream outfile;
    outfile.open("output.txt", std::ios::out | std::ios::app);
    RecordPrinter printer(3);
    printer.print_separator(context);

    for(IndexMeta &index: tab.indexes) {
        std::string rep = "(";
        for(auto &col: index.cols)
            rep += col.name + ",";
        rep.pop_back();
        rep += ")";

        printer.print_record({table_name, "unique", rep}, context);
        outfile << "| " << tab.name << " | unique | " << rep << " |\n";
        printer.print_separator(context);
    }

    outfile.close();
}

std::unique_ptr<RmFileHandle> SmManager::open_table_file(const std::string& table_name) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        std::cout << "当前工作目录: " << cwd << std::endl;
    }
    // 不添加数据库名前缀
    std::cout << "尝试打开表文件: " << table_name << std::endl;
    
    // 检查文件是否存在
    if (access(table_name.c_str(), F_OK) != -1) {
        std::cout << "文件存在: " << table_name << std::endl;
    } else {
        std::cerr << "文件不存在: " << table_name << std::endl;
    }
    
    return std::unique_ptr<RmFileHandle>(
        rm_manager_->open_file(table_name.c_str())
    );
}

void SmManager::get_all_tables(std::vector<std::string>& tables) const {
    for (const auto& pair : db_.tabs_) {
        tables.push_back(pair.first);
    }
}
