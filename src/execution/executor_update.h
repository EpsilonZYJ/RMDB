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
#include "execution_defs.h"
#include "check_condition.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"

class UpdateExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;                       // 表的元和数据
    std::vector<Condition> conds_;      // update的条件(?)
    RmFileHandle *fh_;                  // 表的数据文件句柄
    std::vector<Rid> rids_;             // 各个记录的标识，表示update的位置(?)
    std::string tab_name_;              // 表名称
    std::vector<SetClause> set_clauses_;// 更新语句
    SmManager *sm_manager_;             // 系统管理器，负责元数据管理和DDL语句的执行
   public:
    UpdateExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<SetClause> set_clauses,
                   std::vector<Condition> conds, std::vector<Rid> rids, Context *context) {
        sm_manager_ = sm_manager;
        tab_name_ = tab_name;
        set_clauses_ = set_clauses;
        tab_ = sm_manager_->db_.get_table(tab_name);
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        conds_ = conds;
        rids_ = rids;
        context_ = context;
    }

    // 由于是update，Next()只调用一次（同insert，delete）
    std::unique_ptr<RmRecord> Next() override {
        // 仿照executor_insert的示例
        for(auto& rid: rids_ ) {
            // 读取当前记录
            RmRecord old_record = *fh_->get_record(rid, context_);
            
            // 检查条件是否满足，若不满足则跳过，不进行更新
            // 这里的条件是指update语句中的where条件
            if(!check_condition(old_record, tab_, conds_)) continue;

            // 构造新的记录
            RmRecord new_record(old_record);
            for(auto& set_clause: set_clauses_) {
                auto col_meta = tab_.get_col(set_clause.lhs.col_name);
                int offset = col_meta->offset;
                
                // 检查是否为表达式更新
                if (set_clause.is_expr && 
                    !set_clause.ref_col.col_name.empty()) {
                    
                    // 获取引用列的元数据
                    auto ref_col_meta = tab_.get_col(set_clause.ref_col.col_name);
                    int ref_offset = ref_col_meta->offset;
                    
                    // 创建Value对象表示原始列值
                    Value orig_value;
                    switch (ref_col_meta->type) {
                        case TYPE_INT:
                            orig_value.set_int(*(int*)(old_record.data + ref_offset));
                            break;
                        case TYPE_FLOAT:
                            orig_value.set_float(*(float*)(old_record.data + ref_offset));
                            break;
                        case TYPE_STRING:
                            orig_value.set_str(std::string(old_record.data + ref_offset, ref_col_meta->len));
                            break;
                        default:
                            throw InternalError("不支持的列类型");
                    }
                    
                    // 获取右侧值
                    Value rhs_value = set_clause.rhs;
                    
                    // 确保值类型一致
                    if (orig_value.type != rhs_value.type) {
                        if (value_type_match(orig_value.type, rhs_value.type))
                            rhs_value.value_cast(orig_value.type);
                        else
                            throw IncompatibleTypeError(coltype2str(orig_value.type), 
                                                     coltype2str(rhs_value.type));
                    }
                    
                    // 计算结果
                    Value result_value;
                    switch (set_clause.op_type) {
                        case '+':
                            if (orig_value.type == TYPE_INT)
                                result_value.set_int(orig_value.int_val + rhs_value.int_val);
                            else if (orig_value.type == TYPE_FLOAT)
                                result_value.set_float(orig_value.float_val + rhs_value.float_val);
                            break;
                            
                        case '-':
                            if (orig_value.type == TYPE_INT)
                                result_value.set_int(orig_value.int_val - rhs_value.int_val);
                            else if (orig_value.type == TYPE_FLOAT)
                                result_value.set_float(orig_value.float_val - rhs_value.float_val);
                            break;
                            
                        case '*':
                            if (orig_value.type == TYPE_INT)
                                result_value.set_int(orig_value.int_val * rhs_value.int_val);
                            else if (orig_value.type == TYPE_FLOAT)
                                result_value.set_float(orig_value.float_val * rhs_value.float_val);
                            break;
                            
                        case '/':
                            if (rhs_value.int_val == 0 || rhs_value.float_val == 0.0)
                                throw InternalError("除数不能为零");
                                
                            if (orig_value.type == TYPE_INT)
                                result_value.set_int(orig_value.int_val / rhs_value.int_val);
                            else if (orig_value.type == TYPE_FLOAT)
                                result_value.set_float(orig_value.float_val / rhs_value.float_val);
                            break;
                            
                        default:
                            throw InternalError("不支持的运算符");
                    }
                    
                    // 结果类型转换
                    result_value.init_raw(col_meta->len);
                    memcpy(new_record.data + offset, result_value.raw->data, col_meta->len);
                } else {
                    // 处理普通赋值
                    Value value = set_clause.rhs;
                    if (value.type != col_meta->type) {
                        if (value_type_match(col_meta->type, value.type)) {
                            // 转换类型
                            value.value_cast(col_meta->type);
                          }}
                    if (!value.raw) {
                        value.init_raw(col_meta->len);               
                        // 根据值类型设置raw数据
                        switch (value.type) {
                            case TYPE_INT:
                                *(int*)value.raw->data = value.int_val;
                                break;
                            case TYPE_FLOAT:
                                *(float*)value.raw->data = value.float_val;
                                break;
                            case TYPE_STRING:
                                if (!value.str_val.empty()) {
                                    memcpy(value.raw->data, value.str_val.c_str(), 
                                          std::min(col_meta->len, (int)value.str_val.length()));
                                }
                                break;
                        }
                    }
                    memcpy(new_record.data + offset, value.raw->data, col_meta->len);
                }
                // 日志记录代码
                if (context_ && context_->txn_ && context_->log_mgr_) {
                    // 创建UPDATE日志记录
                    UpdateLogRecord* log_record;
        
                    if (set_clause.is_expr) {
                        // 对于表达式更新，使用扩展的构造函数
                        log_record = new UpdateLogRecord(
                            context_->txn_->get_transaction_id(),
                            old_record,
                            new_record,
                            rid,
                            tab_name_,
                            true,                           // is_expr_update
                            set_clause.op_type,             // 操作符
                            set_clause.ref_col.col_name     // 引用列名
                        );
                    } else {
                        // 对于普通更新，使用原有构造函数
                        log_record = new UpdateLogRecord(
                            context_->txn_->get_transaction_id(),
                            old_record,
                            new_record,
                            rid,
                            tab_name_
                        );
                    }
                    
                    log_record->prev_lsn_ = context_->txn_->get_prev_lsn();
                    // 写日志，获得当前日志lsn
                    lsn_t curr_lsn = context_->log_mgr_->add_log_to_buffer(log_record);
                    // 写日志后，更新事务的prev_lsn
                    context_->txn_->set_prev_lsn(curr_lsn);
                    context_->log_mgr_->flush_log_to_disk();
                    // 释放日志记录内存
                    delete log_record;  
                    //std::cout << "DEBUG: 已生成UPDATE日志记录，表名: " << tab_name_ << ", RID: (" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                }
            }
        
            // 索引更新
            auto **old_keys = new char *[tab_.indexes.size()];
            auto **new_keys = new char *[tab_.indexes.size()];
            auto **ihs = new IxIndexHandle *[tab_.indexes.size()];
            for (size_t i = 0; i < tab_.indexes.size(); ++i) {
                    auto &index = tab_.indexes[i]; // 获取当前遍历到的索引 类型为IndexMeta
                    auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();
                    old_keys[i] = new char[index.col_tot_len];
                    new_keys[i] = new char[index.col_tot_len];
                    ihs[i] = ih; // 存储B+树索引句柄
                    int offset = 0;
                    for (size_t j = 0; j < index.col_num; ++j) {
                        memcpy(old_keys[i] + offset, 
                               old_record.data + index.cols[j].offset, 
                               index.cols[j].len);
                        memcpy(new_keys[i] + offset,
                               new_record.data + index.cols[j].offset, 
                               index.cols[j].len);
                        offset += index.cols[j].len;
                    }
                    if (strcmp(old_keys[i], new_keys[i]) != 0 && !ihs[i]->is_unique(new_keys[i], context_->txn_)) {
                        for (int j = 0; j <= i; ++j) {
                            delete []old_keys[j];
                            delete []new_keys[j];
                        }
                        delete []old_keys;
                        delete []new_keys;
                        delete []ihs;
                        throw IndexNotUniqueError(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols));
                    }
                    //++i;
            }

            // 在更新前，添加写记录到事务写集合
            if (context_ && context_->txn_) {
                // 创建UPDATE类型的写记录，保存原始记录供回滚使用
                // 注意：需要创建一个old_record的副本，因为这里的old_record是栈变量
                RmRecord record_copy(old_record.size);
                memcpy(record_copy.data, old_record.data, old_record.size);
                
                WriteRecord* write_record = new WriteRecord(WType::UPDATE_TUPLE, tab_name_, rid, record_copy);
                context_->txn_->append_write_record(write_record);
                
                //std::cout << "DEBUG:已将更新写记录添加到事务 " << context_->txn_->get_transaction_id()<< ", 写集合大小: " << context_->txn_->get_write_set()->size() << std::endl;
            } else {
                //std::cout << "DEBUG:更新操作没有关联有效事务!" << std::endl;
            }

            fh_->update_record(rid, new_record.data, context_); // 更新数据文件中的记录
           
            for(size_t i = 0; i < tab_.indexes.size(); ++i) {
                ihs[i]->delete_entry(old_keys[i], context_->txn_); 
                ihs[i]->insert_entry(new_keys[i], rid, context_->txn_);
                delete []old_keys[i];
                delete []new_keys[i];
            }
            delete []old_keys;
            delete []new_keys;
            delete []ihs; 
        }

        return nullptr;
    }

    Rid &rid() override { return _abstract_rid; }
};