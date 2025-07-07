/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "transaction_manager.h"
#include "record/rm_file_handle.h"
#include "system/sm_manager.h"

std::unordered_map<txn_id_t, Transaction *> TransactionManager::txn_map = {};

/**
 * @description: 事务的开始方法
 * @return {Transaction*} 开始事务的指针
 * @param {Transaction*} txn 事务指针，空指针代表需要创建新事务，否则开始已有事务
 * @param {LogManager*} log_manager 日志管理器指针
 */
 Transaction * TransactionManager::begin(Transaction* txn, LogManager* log_manager) {
    // 判断传入事务参数是否为空指针
    if (txn != nullptr) {
        // 如果不为空，开始已有事务
        std::cout << "[DEBUG] BEGIN: 开始已有事务，txn_id=" << txn->get_transaction_id() << std::endl;
        // 检查事务是否已经在事务表中
        std::unique_lock<std::mutex> lock(latch_);
        auto it = txn_map.find(txn->get_transaction_id());
        if (it != txn_map.end()) {
            return txn;
        } else {
            txn_map[txn->get_transaction_id()] = txn;
            return txn;
        }
    }
    
    txn_id_t txn_id = next_txn_id_++;
    std::cout << "[DEBUG] BEGIN: 创建新事务，txn_id=" << txn_id << std::endl;
    Transaction* new_txn = new Transaction(txn_id);
    new_txn->set_state(TransactionState::GROWING);
    new_txn->set_txn_mode(false);  // ✅ 修复：默认设为隐式事务
    
    {
        std::unique_lock<std::mutex> lock(latch_);
        txn_map[txn_id] = new_txn;
    }
    
    // 写日志
    if (log_manager != nullptr) {
        BeginLogRecord log_record(txn_id);
        log_record.prev_lsn_ = INVALID_LSN;  
        lsn_t lsn = log_manager->add_log_to_buffer(&log_record); 
        new_txn->set_prev_lsn(lsn);
    }
    return new_txn;
}

/**
 * @description: 事务的提交方法
 * @param {Transaction*} txn 需要提交的事务
 * @param {LogManager*} log_manager 日志管理器指针
 */
void TransactionManager::commit(Transaction* txn, LogManager* log_manager) {
    // Todo:
    // 1. 如果存在未提交的写操作，提交所有的写操作
    // 2. 释放所有锁
    // 3. 释放事务相关资源，eg.锁集
    // 4. 把事务日志刷入磁盘中
    // 5. 更新事务状态
    // 如果需要支持MVCC请在上述过程中添加代码  
    if (txn == nullptr) return;
    
    // 写提交日志
    if (log_manager != nullptr) {
        CommitLogRecord log_record(txn->get_transaction_id());
        log_record.prev_lsn_ = txn->get_prev_lsn();
        lsn_t curr_lsn = log_manager->add_log_to_buffer(&log_record);
        txn->set_prev_lsn(curr_lsn);
    }
    
    // 清理写集合
    for(auto it = txn->get_write_set()->begin(); it != txn->get_write_set()->end();) {
        delete *it;
        it = txn->get_write_set()->erase(it);
    }
    
    // 释放所有锁
    for(auto it = txn->get_lock_set()->begin(); it != txn->get_lock_set()->end();) {
        lock_manager_->unlock(txn, *it);
        it = txn->get_lock_set()->erase(it);
    }
    
    // 刷新日志
    if (log_manager != nullptr) {
        log_manager->flush_log_to_disk();
    }
    
    // 设置状态
    txn->set_state(TransactionState::COMMITTED);
    
    // 从事务表移除
    {
        std::unique_lock<std::mutex> lock(latch_);
        txn_map.erase(txn->get_transaction_id());
    }
}

/**
 * @description: 事务的终止（回滚）方法
 * @param {Transaction *} txn 需要回滚的事务
 * @param {LogManager} *log_manager 日志管理器指针
 */
 void TransactionManager::abort(Transaction * txn, LogManager *log_manager) {
    if (txn == nullptr) return;
    
    std::cout << "开始回滚事务: " << txn->get_transaction_id() << std::endl;
    
    // 写回滚日志
    if (log_manager != nullptr) {
        AbortLogRecord log_record(txn->get_transaction_id());
        log_record.prev_lsn_ = txn->get_prev_lsn();
        lsn_t curr_lsn = log_manager->add_log_to_buffer(&log_record);
        txn->set_prev_lsn(curr_lsn);
    }
    
    // 回滚所有写操作 - 使用LIFO顺序
    auto write_set = txn->get_write_set();
    while (!write_set->empty()) {
        WriteRecord *write_record = write_set->back();  
        write_set->pop_back();                          
        
        try {
            std::string tab_name = write_record->GetTableName();
            
            // 检查表是否存在
            if (sm_manager_->fhs_.find(tab_name) == sm_manager_->fhs_.end()) {
                std::cout << "警告：表 " << tab_name << " 不存在，跳过回滚" << std::endl;
                delete write_record;
                continue;
            }
            
            auto fh = sm_manager_->fhs_.at(tab_name).get();
            WType type = write_record->GetWriteType();
            
            std::cout << "回滚操作: 表=" << tab_name 
                      << ", 类型=" << static_cast<int>(type)
                      << ", RID=(" << write_record->GetRid().page_no 
                      << "," << write_record->GetRid().slot_no << ")" << std::endl;
            
            switch (type) {
                case WType::INSERT_TUPLE:
                    // 回滚INSERT：删除记录
                    fh->delete_record(write_record->GetRid(), nullptr);
                    break;
                    
                case WType::DELETE_TUPLE:
                    // 回滚DELETE：重新插入记录
                    fh->insert_record(write_record->GetRid(), write_record->GetRecord().data);
                    break;
                    
                case WType::UPDATE_TUPLE:
                    // 回滚UPDATE：恢复原记录
                    fh->update_record(write_record->GetRid(), write_record->GetRecord().data, nullptr);
                    break;
                    
                default:
                    break;
            }
            
            // ✅ 简化的索引回滚 - 重建所有索引
            if (sm_manager_->db_.is_table(tab_name)) {
                auto& tab = sm_manager_->db_.get_table(tab_name);
                for (auto& index : tab.indexes) {
                    try {
                        std::vector<std::string> index_cols;
                        for (auto& col : index.cols) {
                            index_cols.push_back(col.name);
                        }
                        // 重建索引（简单粗暴但有效）
                        sm_manager_->drop_index(tab_name, index.cols, nullptr);
                        sm_manager_->create_index(tab_name, index_cols, nullptr);
                        std::cout << "重建索引: " << tab_name << std::endl;
                    } catch (const std::exception& e) {
                        std::cout << "重建索引失败: " << e.what() << std::endl;
                    }
                }
            }
            
        } catch (const std::exception& e) {
            std::cout << "回滚操作失败: " << e.what() << std::endl;
        }
        
        delete write_record;
    }
    
    // 释放所有锁
    for(auto it = txn->get_lock_set()->begin(); it != txn->get_lock_set()->end();) {
        lock_manager_->unlock(txn, *it);
        it = txn->get_lock_set()->erase(it);
    }
    
    // 刷新日志
    if (log_manager != nullptr) {
        log_manager->flush_log_to_disk();
    }
    
    // 更新事务状态
    txn->set_state(TransactionState::ABORTED);
    
    // 从事务表移除
    {
        std::unique_lock<std::mutex> lock(latch_);
        txn_map.erase(txn->get_transaction_id());
    }
    
    std::cout << "事务回滚完成: " << txn->get_transaction_id() << std::endl;
}


