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

            // 进行更新
            RmRecord new_record(old_record);
            for(auto& set_clauses: set_clauses_) {
                auto col_meta = tab_.get_col(set_clauses.lhs.col_name);
                int offset = col_meta->offset;
                Value &value = set_clauses.rhs;

                memcpy(new_record.data + offset, value.raw->data, col_meta->len);
            }

            // // 更新索引前先删除旧索引
            // auto indexes = tab_.get_indexes(); // Retrieve indexes from the table metadata
            // for(auto &index: indexes) {
            //     // 获取索引句柄
            //     auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();
            //     // 创建旧记录的索引键
            //     auto old_key = ix::IxManager::create_key(old_record.data, index);
            //     // 从索引中删除旧记录
            //     ih->delete_entry(old_key.get(), context_->txn_);
            // }
        
            fh_->update_record(rid, new_record.data, context_); // 更新数据文件中的记录

            //TODO 增加索引更新

        }
        return nullptr;
    }

    Rid &rid() override { return _abstract_rid; }
};