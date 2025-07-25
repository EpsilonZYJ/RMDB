/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "execution_manager.h"

#include "executor_delete.h"
#include "executor_index_scan.h"
#include "executor_insert.h"
#include "executor_nestedloop_join.h"
#include "executor_projection.h"
#include "executor_seq_scan.h"
#include "executor_update.h"
#include "index/ix.h"
#include "record_printer.h"
#include "execution/executor_explain.h"
#include "executor_semi_join.h"
const char *help_info = "Supported SQL syntax:\n"
                   "  command ;\n"
                   "command:\n"
                   "  CREATE TABLE table_name (column_name type [, column_name type ...])\n"
                   "  DROP TABLE table_name\n"
                   "  CREATE INDEX table_name (column_name)\n"
                   "  DROP INDEX table_name (column_name)\n"
                   "  INSERT INTO table_name VALUES (value [, value ...])\n"
                   "  DELETE FROM table_name [WHERE where_clause]\n"
                   "  UPDATE table_name SET column_name = value [, column_name = value ...] [WHERE where_clause]\n"
                   "  SELECT selector FROM table_name [WHERE where_clause]\n"
                   "type:\n"
                   "  {INT | FLOAT | CHAR(n)}\n"
                   "where_clause:\n"
                   "  condition [AND condition ...]\n"
                   "condition:\n"
                   "  column op {column | value}\n"
                   "column:\n"
                   "  [table_name.]column_name\n"
                   "op:\n"
                   "  {= | <> | < | > | <= | >=}\n"
                   "selector:\n"
                   "  {* | column [, column ...]}\n";

// 主要负责执行DDL语句
void QlManager::run_mutli_query(std::shared_ptr<Plan> plan, Context *context,txn_id_t *txn_id){
    if (auto x = std::dynamic_pointer_cast<CreateCheckpointPlan>(plan)) {
        // 创建静态检查点
        log_manager_->create_static_checkpoint(txn_mgr_, buffer_pool_manager_, sm_manager_);
        std::cout << "静态检查点创建成功" << std::endl;
    }
    if (auto x = std::dynamic_pointer_cast<DDLPlan>(plan)) {
        switch(x->tag) {
            case T_CreateTable:
            {
                sm_manager_->create_table(x->tab_name_, x->cols_, context);
                break;
            }
            case T_DropTable:
            {
                sm_manager_->drop_table(x->tab_name_, context);
                break;
            }
            case T_CreateIndex:
            {
                sm_manager_->create_index(x->tab_name_, x->tab_col_names_, context);
                break;
            }
            case T_DropIndex:
            {
                sm_manager_->drop_index(x->tab_name_, x->tab_col_names_, context);
                break;
            }
            default:
                throw InternalError("Unexpected field type");
                break;  
        }
    }
    if (context->txn_ && !context->txn_->get_txn_mode()) {
        txn_mgr_->commit(context->txn_, context->log_mgr_);
        context->txn_ = nullptr;
        if (txn_id) *txn_id = INVALID_TXN_ID;
    }
}

// 执行help; show tables; desc table; begin; commit; abort;语句
void QlManager::run_cmd_utility(std::shared_ptr<Plan> plan, txn_id_t *txn_id, Context *context) {
    if (auto x = std::dynamic_pointer_cast<OtherPlan>(plan)) {
        switch(x->tag) {
            case T_Help:
            {
                memcpy(context->data_send_ + *(context->offset_), help_info, strlen(help_info));
                *(context->offset_) = strlen(help_info);
                break;
            }
            case T_ShowTable:
            {
                sm_manager_->show_tables(context);
                break;
            }
            case T_DescTable:
            {
                sm_manager_->desc_table(x->tab_name_, context);
                break;
            }
            case T_Transaction_begin:
            {
                // 显示开启一个事务
                if (context->txn_ != nullptr) {
                    context->txn_->set_txn_mode(true);
                    *txn_id = context->txn_->get_transaction_id();
                    std::cout << "设置事务为显式模式: ID=" << *txn_id << std::endl;
                }
                break;
            }  
            case T_Transaction_commit:
            {
                if (context->txn_ != nullptr) {
                    txn_mgr_->commit(context->txn_, context->log_mgr_);
                    *txn_id = INVALID_TXN_ID;
                }
                break;
            }    
            case T_Transaction_rollback:
            {
                if (context->txn_ != nullptr) {
                    txn_mgr_->abort(context->txn_, context->log_mgr_);
                    *txn_id = INVALID_TXN_ID;
                }
                break;
            }    
            case T_Transaction_abort:
            {
                if (context->txn_ != nullptr) {
                    txn_mgr_->abort(context->txn_, context->log_mgr_);
                    *txn_id = INVALID_TXN_ID;  // 重置事务ID
                }
                break;
            }     
            case T_ShowIndex:
            {
                sm_manager_->show_index(x->tab_name_, context);
                break;
            }
            default:
                throw InternalError("Unexpected field type");
                break;                        
        }

    } else if(auto x = std::dynamic_pointer_cast<SetKnobPlan>(plan)) {
        switch (x->set_knob_type_)
        {
        case ast::SetKnobType::EnableNestLoop: {
            planner_->set_enable_nestedloop_join(x->bool_value_);
            break;
        }
        case ast::SetKnobType::EnableSortMerge: {
            planner_->set_enable_sortmerge_join(x->bool_value_);
            break;
        }
        default: {
            throw RMDBError("Not implemented!\n");
            break;
        }
        }
    }
}

// 执行select语句，select语句的输出除了需要返回客户端外，还需要写入output.txt文件中
void QlManager::select_from(std::unique_ptr<AbstractExecutor> executorTreeRoot, std::vector<TabCol> sel_cols, 
                            Context *context) {
    std::cout<<"DEBUG: select_from 开始" << std::endl;
    // 检查是否为EXPLAIN执行器
    if (dynamic_cast<ExplainExecutor*>(executorTreeRoot.get())) {
        // 特殊处理EXPLAIN结果
        executorTreeRoot->beginTuple();
        auto record = executorTreeRoot->Next();
        if (record) {
            // 直接写入文件
            std::fstream outfile;
            outfile.open("output.txt", std::ios::out | std::ios::app);
            outfile << record->data << std::endl;
            outfile.close();
            
            // 设置上下文响应
            memcpy(context->data_send_ + *(context->offset_), 
                   record->data, strlen(record->data));
            *(context->offset_) += strlen(record->data);
        }
        return;
    }
    std::vector<std::string> captions;
    captions.reserve(sel_cols.size());
    for (auto &sel_col : sel_cols) {
        std::cout << "DEBUG: select_from sel_col.col_name: " << sel_col.col_name << std::endl;
        captions.push_back(sel_col.col_name);
    }

    // Print header into buffer
    RecordPrinter rec_printer(sel_cols.size());
    rec_printer.print_separator(context);
    rec_printer.print_record(captions, context);
    rec_printer.print_separator(context);
    // print header into file
    std::fstream outfile;
    outfile.open("output.txt", std::ios::out | std::ios::app);
    outfile << "|";
    for(int i = 0; i < captions.size(); ++i) {
        outfile << " " << captions[i] << " |";
    }
    outfile << "\n";

    // Print records
    size_t num_rec = 0;
    // 执行query_plan
    for (executorTreeRoot->beginTuple(); !executorTreeRoot->is_end(); executorTreeRoot->nextTuple()) {
        auto Tuple = executorTreeRoot->Next();
        std::vector<std::string> columns;
        for (auto &col : executorTreeRoot->cols()) {
            std::string col_str;
            char *rec_buf = Tuple->data + col.offset;
            if (col.type == TYPE_INT) {
                col_str = std::to_string(*(int *)rec_buf);
            } else if (col.type == TYPE_FLOAT) {
                col_str = std::to_string(*(float *)rec_buf);
            } else if (col.type == TYPE_STRING) {
                col_str = std::string((char *)rec_buf, col.len);
                col_str.resize(strlen(col_str.c_str()));
            } else if (col.type == TYPE_DATE) { 
                // Date is stored as a string in the record
                col_str = std::string((char *)rec_buf, col.len);
                col_str.resize(strlen(col_str.c_str()));
            } else {
                throw IncompatibleTypeError(coltype2str(col.type), "Unknown");
            }
            columns.push_back(col_str);
        }
        // print record into buffer
        rec_printer.print_record(columns, context);
        // print record into file
        outfile << "|";
        for(int i = 0; i < columns.size(); ++i) {
            outfile << " " << columns[i] << " |";
        }
        outfile << "\n";
        num_rec++;
    }
    outfile.close();
    // Print footer into buffer
    rec_printer.print_separator(context);
    // Print record count into buffer
    RecordPrinter::print_record_count(num_rec, context);
    std::cout<<"DEBUG: select_from 结束" << std::endl;
}

// 执行DML语句
void QlManager::run_dml(std::unique_ptr<AbstractExecutor> exec){
    exec->Next();
    
    // 添加自动提交逻辑
    if (exec->context_ && exec->context_->txn_ && !exec->context_->txn_->get_txn_mode()) {
        // 只有在非显式事务模式下才自动提交
        std::cout << "自动提交隐式事务: " << exec->context_->txn_->get_transaction_id() << std::endl;
        
        // 使用事务管理器正确提交，而不是手动创建日志
        try {
            txn_mgr_->commit(exec->context_->txn_, exec->context_->log_mgr_);
            std::cout << "隐式事务提交成功" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "隐式事务提交失败: " << e.what() << std::endl;
            // 提交失败时尝试回滚
            try {
                txn_mgr_->abort(exec->context_->txn_, exec->context_->log_mgr_);
            } catch (...) {
                std::cerr << "回滚也失败" << std::endl;
            }
        }
    }
}

void QlManager::load_data(const std::string& file_name, const std::string& table_name, Transaction* txn) {
   std::ifstream fin(file_name);
if (!fin.is_open()) throw std::runtime_error("无法打开文件: " + file_name);

bool auto_txn = false;
if (!txn) {
    txn = txn_mgr_->begin(nullptr, log_manager_);
    auto_txn = true;
}
int cnt = 0;
Context ctx(lock_manager_, log_manager_, txn, nullptr, nullptr);

TabMeta tab = sm_manager_->db_.get_table(table_name);
std::string line;

// 跳过表头
bool first_line = true;
while (std::getline(fin, line)) {
    if (line.empty()) continue;
    if (first_line) { first_line = false; continue; } // 跳过表头

    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    if (fields.size() != tab.cols.size()) {
        throw std::runtime_error("字段数不匹配: " + line);
    }
    std::vector<Value> values;
    for (size_t i = 0; i < fields.size(); ++i) {
        Value v;
        switch (tab.cols[i].type) {
            case TYPE_INT:
                v.set_int(std::stoi(fields[i]));
                break;
            case TYPE_FLOAT:
                v.set_float(std::stof(fields[i]));
                break;
            case TYPE_STRING:
                v.set_str(fields[i]);
                break;
            case TYPE_DATE:
                v.set_date(fields[i]);
                break;
            default:
                throw std::runtime_error("未知字段类型");
        }
        values.push_back(v);
    }
    InsertExecutor exec(sm_manager_, table_name, values, &ctx);
    exec.Next();
    cnt++;
}
if (auto_txn) {
    txn_mgr_->commit(txn, log_manager_);
}
std::cout << "Load " << cnt << " rows into " << table_name << std::endl;
}