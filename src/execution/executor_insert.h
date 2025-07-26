/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once
#define DEBUG 1
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"

class InsertExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;                   // 表的元数据
    std::vector<Value> values_;     // 需要插入的数据
    RmFileHandle *fh_;              // 表的数据文件句柄
    std::string tab_name_;          // 表名称
    Rid rid_;                       // 插入的位置，由于系统默认插入时不指定位置，因此当前rid_在插入后才赋值
    SmManager *sm_manager_;         // 系统管理器，负责元数据管理和DDL语句的执行

   public:
    /**
     * @description: 构造函数
     * @param sm_manager 系统管理器
     * @param tab_name 要进行插入操作的表的名称
     * @param values 要插入的值
     * @param context 查询上下文
     */
    InsertExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<Value> values, Context *context) {
        sm_manager_ = sm_manager;
        tab_ = sm_manager_->db_.get_table(tab_name); // 根据给定的表名获取对应的表的元素数据
        values_ = values;
        tab_name_ = tab_name;
        if (values.size() != tab_.cols.size()) { // 判断要插入的值的个数是否与表的列数是否相同
            throw InvalidValueCountError();
        }
        fh_ = sm_manager_->fhs_.at(tab_name).get(); // 获取给定文件对应的RmFileHandler
        context_ = context;
    };

    std::unique_ptr<RmRecord> Next() override {
        // Make record buffer
        RmRecord rec(fh_->get_file_hdr().record_size);
        for (size_t i = 0; i < values_.size(); i++) { // 依次遍历每个待插入的值
            auto &col = tab_.cols[i]; // 类型为ColMeta
            auto &val = values_[i];  //（父）类型为Value
            
            // 特殊处理日期类型
            if (col.type == TYPE_DATE && val.type == TYPE_STRING) {
                // 如果是日期类型且值是字符串，则将字符串转换为日期格式
                Date date(val.str_val);
                val.set_date(date);
            }

            if (!value_type_match(col.type, val.type)){ 
                throw IncompatibleTypeError(coltype2str(col.type), coltype2str(val.type));
            }
            if(col.type != val.type) {
                val.value_cast(col.type); // 确保新值与字段类型匹配
            }
            val.init_raw(col.len); // 用特定的值初始化
            memcpy(rec.data + col.offset, val.raw->data, col.len);
        }

        // Insert into index
        auto ihs = new IxIndexHandle *[tab_.indexes.size()]; // 创建一个B+树索引句柄数组，大小为表的索引数量
        auto keys = new char* [tab_.indexes.size()];
        for(size_t i = 0; i < tab_.indexes.size(); ++i) { // 依次遍历表的所有索引
            auto& index = tab_.indexes[i]; // 获取当前遍历到的索引 类型为IndexMeta
            auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get(); // 获取对应的B+树，类型为IxNodeHandle
            char* key = new char[index.col_tot_len]; // col_tot_len表示索引字段长度总和
            int offset = 0;
            // 下面构造用于B+树索引的key
            for(size_t j = 0; j < index.col_num; ++j) { // col_num表示索引字段数量
                memcpy(key + offset, rec.data + index.cols[j].offset, index.cols[j].len);
                offset += index.cols[j].len;
            }
#if DEBUG
            static int cnt;
            cnt ++;
            if(cnt == 2)
            cnt = cnt ;
#endif
            if(ih->is_unique(key, context_->txn_)) {// 检查有没有键的重复
                ihs[i] = ih; 
                keys[i] = key; // 将key存储到keys数组中
            } else {
                for(int k = 0; k < i; k++) delete[] keys[k];
                delete[] keys;
                delete[] ihs;
                throw IndexNotUniqueError(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols));
            }
        }

        // Insert into record file
        rid_ = fh_->insert_record(rec.data, context_); 
        // 增加表的元组计数
        sm_manager_->db_.get_table(tab_name_).increment_tuple_count();
        // 添加写记录到事务写集合
        if (context_ && context_->txn_) {
            // 创建写记录
            WriteRecord* write_record = new WriteRecord(WType::INSERT_TUPLE, tab_name_,rid_, rec);
            // 添加到事务写集合
            context_->txn_->append_write_record(write_record);
            if (context_ && context_->txn_ && context_->log_mgr_) {
                // 创建INSERT日志记录
                InsertLogRecord* log_record = new InsertLogRecord(
                    context_->txn_->get_transaction_id(),
                    rec,
                    rid_,
                    tab_name_
                );
                // 追加日志记录
               // 维护prev_lsn链：写日志前设置prev_lsn
                log_record->prev_lsn_ = context_->txn_->get_prev_lsn();
                // 写日志，获得当前日志lsn
                lsn_t curr_lsn = context_->log_mgr_->add_log_to_buffer(log_record);
                // 写日志后，更新事务的prev_lsn
                context_->txn_->set_prev_lsn(curr_lsn);
                context_->log_mgr_->flush_log_to_disk();
                // 释放日志记录内存
                delete log_record;
                
                //std::cout << "DEBUG: 已生成INSERT日志记录，表名: " << tab_name_ << ", RID: (" << rid_.page_no << "," << rid_.slot_no << ")" << std::endl;
            }
            //std::cout << "DEBUG:已将插入写记录添加到事务 " << context_->txn_->get_transaction_id() << ", 写集合大小: " << context_->txn_->get_write_set()->size() << std::endl;
        } else {
            std::cout << "DEBUG:插入操作没有关联有效事务!" << std::endl;
        }
        
        for(size_t i = 0; i < tab_.indexes.size(); ++i) { // 依次遍历表的所有索引
            ihs[i]->insert_entry(keys[i], rid_, context_->txn_); // 将当前记录插入到B+树中
            delete[] keys[i]; // 释放key的内存
        }
        delete []keys;
        delete []ihs; // 释放B+树索引句柄数组的内存
        
        return nullptr;
    }
    Rid &rid() override { return rid_; }
};