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
    // Todo:
    // 1. 判断传入事务参数是否为空指针
    // 2. 如果为空指针，创建新事务
    // 3. 把开始事务加入到全局事务表中
    // 4. 返回当前事务指针
    // 如果需要支持MVCC请在上述过程中添加代码
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
    new_txn->set_txn_mode(true);  // 默认显式事务
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
    // Todo:
    // 1. 回滚所有写操作
    // 2. 释放所有锁
    // 3. 清空事务相关资源，eg.锁集
    // 4. 把事务日志刷入磁盘中
    // 5. 更新事务状态
    if (txn == nullptr) return;
    std::cout << "1" << std::endl;
    // 写回滚日志
    if (log_manager != nullptr) {
        AbortLogRecord log_record(txn->get_transaction_id());
        log_record.prev_lsn_ = txn->get_prev_lsn();
        lsn_t curr_lsn = log_manager->add_log_to_buffer(&log_record);
        txn->set_prev_lsn(curr_lsn);
    }
    std::cout << "2" << std::endl;
    // 回滚所有写操作 - 使用LIFO顺序
    Context context(lock_manager_, log_manager, txn);
    auto write_set = txn->get_write_set();
    while (!write_set->empty()) {
        WriteRecord *write_record = write_set->back();  
        write_set->pop_back();                          
            std::string tab_name = write_record->GetTableName();
            
            // 检查表是否存在
            if (sm_manager_->fhs_.find(tab_name) == sm_manager_->fhs_.end()) {
                delete write_record;
                continue;
            }
            
            auto& tab = sm_manager_->db_.get_table(tab_name);
            auto fh = sm_manager_->fhs_.at(tab_name).get();
            WType type = write_record->GetWriteType();
            
            // 先处理DELETE_TUPLE的数据恢复
            if (type == WType::DELETE_TUPLE) {
                 fh->insert_record(write_record->GetRid(), write_record->GetRecord().data, &context);
            }  
            // 处理索引回滚
            for (auto &index : tab.indexes) {
                    std::string index_name = sm_manager_->get_ix_manager()->get_index_name(tab_name, index.cols);
                    if (sm_manager_->ihs_.find(index_name) == sm_manager_->ihs_.end()) {
                        continue;
                    }
                    auto ih = sm_manager_->ihs_.at(index_name).get();
                    
                    // 使用智能指针管理内存
                    std::unique_ptr<char[]> key_ptr(new char[index.col_tot_len]);
                    char *key = key_ptr.get();
                    memset(key, 0, index.col_tot_len);
                    
                    int offset = 0;
                    char* record = write_record->GetRecord().data;
                    std::unique_ptr<RmRecord> rec;
                    
                    // 获取正确的记录数据
                    if (type == WType::INSERT_TUPLE || type == WType::UPDATE_TUPLE) {
                        rec = fh->get_record(write_record->GetRid(), &context);
                        record = rec->data;
                    }
                    
                    // 构建索引键
                    bool key_valid = true;
                    for (size_t j = 0; j < (size_t)index.col_num && key_valid; ++j) {
                        if (index.cols[j].offset + index.cols[j].len <= write_record->GetRecord().size) {
                            memcpy(key + offset, record + index.cols[j].offset, index.cols[j].len);
                            offset += index.cols[j].len;
                        } else {
                            key_valid = false;
                        }
                    }
                    
                    if (!key_valid) continue;
                    
                    // 处理不同类型的索引操作
                    switch (type) {
                    case WType::INSERT_TUPLE:
                        ih->delete_entry(key, txn);
                        break;
                    case WType::DELETE_TUPLE:
                        ih->insert_entry(key, write_record->GetRid(), txn);
                        break;
                    case WType::UPDATE_TUPLE:
                        {
                            // 删除新索引
                            ih->delete_entry(key, txn);
                            
                            // 重建旧索引键
                            std::unique_ptr<char[]> old_key_ptr(new char[index.col_tot_len]);
                            char* old_key = old_key_ptr.get();
                            memset(old_key, 0, index.col_tot_len);
                            
                            char* old_rec = write_record->GetRecord().data;
                            offset = 0;
                            bool old_key_valid = true;
                            for (size_t j = 0; j < (size_t)index.col_num && old_key_valid; ++j) {
                                if (index.cols[j].offset + index.cols[j].len <= write_record->GetRecord().size) {
                                    memcpy(old_key + offset, old_rec + index.cols[j].offset, index.cols[j].len);
                                    offset += index.cols[j].len;
                                } else {
                                    old_key_valid = false;
                                }
                            }
                            
                            if (old_key_valid) {
                                ih->insert_entry(old_key, write_record->GetRid(), txn);
                            }
                        }
                        break;
                    default:
                        break;
                    }
            }
            
            // 处理数据表回滚
            switch (type) {
            case WType::INSERT_TUPLE:
                fh->delete_record(write_record->GetRid(), &context);
                break;
            case WType::UPDATE_TUPLE:
                fh->update_record(write_record->GetRid(), write_record->GetRecord().data,&context);
                break;
            default:
                break;
            }
        
        delete write_record;
    }
    std::cout << "3" << std::endl;
    // 释放所有锁
    for(auto it = txn->get_lock_set()->begin(); it != txn->get_lock_set()->end();) {
        lock_manager_->unlock(txn, *it);
        it = txn->get_lock_set()->erase(it);
    }
    std::cout << "4" << std::endl;
    // 刷新日志
    if (log_manager != nullptr) {
        log_manager->flush_log_to_disk();
    }
    std::cout << "5" << std::endl;
    // 更新事务状态
    txn->set_state(TransactionState::ABORTED);
    
    // 从事务表移除
    {
        std::unique_lock<std::mutex> lock(latch_);
        txn_map.erase(txn->get_transaction_id());
    }

}




