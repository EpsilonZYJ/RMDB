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
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"
#include "check_condition.h"

class DeleteExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;                   // 表的元数据
    std::vector<Condition> conds_;  // delete的条件
    RmFileHandle *fh_;              // 表的数据文件句柄
    std::vector<Rid> rids_;         // 需要删除的记录的位置
    std::string tab_name_;          // 表名称
    SmManager *sm_manager_;

   public:
    DeleteExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<Condition> conds,
                   std::vector<Rid> rids, Context *context) {
        sm_manager_ = sm_manager;
        tab_name_ = tab_name;
        tab_ = sm_manager_->db_.get_table(tab_name);
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        conds_ = conds;
        rids_ = rids;
        context_ = context;
    }

    std::unique_ptr<RmRecord> Next() override {
        // 仿照executor_insert的示例
        for(auto& rid: rids_ ) {
            // 读取当前记录
            RmRecord old_record = *fh_->get_record(rid, context_);
            
            // 检查条件是否满足，若不满足则跳过，不进行更新
            // 这里的条件是指update语句中的where条件
            if(!check_condition(old_record, tab_, conds_)) continue;
            
            // 在删除前，添加写记录到事务写集合
            if (context_ && context_->txn_) {
                // 创建DELETE类型的写记录，保存原始记录供回滚使用
                // 创建一个old_record的副本
                RmRecord record_copy(old_record.size);
                memcpy(record_copy.data, old_record.data, old_record.size);
                
                WriteRecord* write_record = new WriteRecord(WType::DELETE_TUPLE, tab_name_, rid, record_copy);
                context_->txn_->append_write_record(write_record);
                
                std::cout << "DEBUG:已将删除写记录添加到事务 " << context_->txn_->get_transaction_id() 
                        << ", 写集合大小: " << context_->txn_->get_write_set()->size() << std::endl;
            } else {
                std::cout << "DEBUG:删除操作没有关联有效事务!" << std::endl;
            }


            bool debug = fh_->is_record(rid);
            fh_->delete_record(rid, context_); // 更新数据文件中的记录

            for (size_t i = 0; i < tab_.indexes.size(); i++) {
                auto& index = tab_.indexes[i]; // 获取当前遍历到的索引 类型为IndexMeta
                auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get(); // 获取对应的B+树，类型为IxNodeHandl
                // 下面构造用于B+树索引的key
                char *key = new char[index.col_tot_len];
                int offset = 0;
                for(size_t j = 0; j < index.col_num; ++j) { // col_num表示索引字段数量
                    memcpy(key + offset, old_record.data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                // 删除B+树索引中的记录
                ih->delete_entry(key, context_->txn_);
                delete []key;
            }
        }
        return nullptr;
    }

    Rid &rid() override { return _abstract_rid; }
};