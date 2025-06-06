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
    if (txn == nullptr) {
        // 如果为空指针，创建新事务
        txn_id_t txn_id = next_txn_id_++;
        txn = new Transaction(txn_id);
        txn->set_state(TransactionState::GROWING);
    }
    
    // 把开始事务加入到全局事务表中
    {
        std::unique_lock<std::mutex> lock(latch_);
        txn_map[txn->get_transaction_id()] = txn;
    }
    
    // 写入BEGIN日志记录
    if (log_manager != nullptr) {
        try {
            BeginLogRecord log_record(txn->get_transaction_id());
            log_record.prev_lsn_ = txn->get_prev_lsn();
            lsn_t lsn = log_manager->add_log_to_buffer(&log_record);
            txn->set_prev_lsn(lsn);
        } catch (const std::exception& e) {
            // 处理日志错误
            std::cerr << "Warning: Failed to write begin log: " << e.what() << std::endl;
        }
    }
    
    // 4. 返回当前事务指针
    return txn;
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
    // 如果存在未提交的写操作，提交所有的写操作
    auto write_set = txn->get_write_set();
    while (!write_set->empty()) {
        WriteRecord* record = write_set->front();
        write_set->pop_front();
        delete record;
    }
    
    // 释放所有锁
    auto lock_set = txn->get_lock_set();
    for (auto lock_id : *lock_set) {
        lock_manager_->unlock(txn, lock_id);
    }
    
    // 释放事务相关资源
    lock_set->clear();
    
    // 把事务日志刷入磁盘中
    bool log_success = true;
    if (log_manager != nullptr) {
        try {
            CommitLogRecord log_record(txn->get_transaction_id());
            log_record.prev_lsn_ = txn->get_prev_lsn();
            lsn_t lsn = log_manager->add_log_to_buffer(&log_record);
            txn->set_prev_lsn(lsn);
            log_manager->flush_log_to_disk();
        } catch (const std::exception& e) {
            std::cerr << "Error: Failed to write commit log: " << e.what() << std::endl;
            log_success = false;
        }
    }
    
    // 更新事务状态
    txn->set_state(TransactionState::COMMITTED);
    
    // 从事务表中移除
    if (log_success) {
        std::unique_lock<std::mutex> lock(latch_);
        txn_map.erase(txn->get_transaction_id());
    } else {
        // 如果日志失败，保留事务以便后续尝试
        std::cerr << "Warning: Transaction remains in txn_map due to log failure" << std::endl;
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
    // 如果需要支持MVCC请在上述过程中添加代码
     // 回滚所有写操作
     auto write_set = txn->get_write_set();
     int i=0;
     while (!write_set->empty()) {
         WriteRecord* record = write_set->back(); // 从后向前回滚
         write_set->pop_back();
         
         std::string& tab_name = record->GetTableName();
         std::cout << "DEBUG: 回滚操作 #" << ++i
                  << ", 类型: " << (record->GetWriteType() == WType::INSERT_TUPLE ? "INSERT" : 
                                    record->GetWriteType() == WType::DELETE_TUPLE ? "DELETE" : "UPDATE")
                  << ", 表: " << tab_name
                  << ", RID: (" << record->GetRid().page_no << "," << record->GetRid().slot_no << ")"
                  << std::endl;
         RmFileHandle* fh = sm_manager_->fhs_.at(tab_name).get();
         
         // 根据写操作类型执行回滚
         switch (record->GetWriteType()) {
             case WType::INSERT_TUPLE:
                 // 回滚插入：删除该记录
                 std::cout << "DEBUG: 尝试删除已插入记录" << std::endl;
                 fh->delete_record(record->GetRid(), nullptr);
                 std::cout << "DEBUG: 记录删除成功" << std::endl;
                 break;
                 
             case WType::DELETE_TUPLE:
                 // 回滚删除：重新插入记录
                 fh->insert_record(record->GetRid(), record->GetRecord().data);
                 break;
                 
             case WType::UPDATE_TUPLE:
                 // 回滚更新：恢复原数据
                 fh->update_record(record->GetRid(), record->GetRecord().data, nullptr);
                 break;
         }
         
         delete record;
     }
     
     // 释放所有锁
     auto lock_set = txn->get_lock_set();
     for (auto lock_id : *lock_set) {
         lock_manager_->unlock(txn, lock_id);
     }
     
     // 清空事务相关资源
     lock_set->clear();
     
     // 把事务日志刷入磁盘中
     bool log_success = true;
     if (log_manager != nullptr) {
         try {
             AbortLogRecord log_record(txn->get_transaction_id());
             log_record.prev_lsn_ = txn->get_prev_lsn();
             lsn_t lsn = log_manager->add_log_to_buffer(&log_record);
             txn->set_prev_lsn(lsn);
             log_manager->flush_log_to_disk();
         } catch (const std::exception& e) {
             std::cerr << "Error: Failed to write abort log: " << e.what() << std::endl;
             log_success = false;
         }
     }
     
     // 更新事务状态
     txn->set_state(TransactionState::ABORTED);
     
     // 从事务表中移除
     if (log_success) {
         std::unique_lock<std::mutex> lock(latch_);
         txn_map.erase(txn->get_transaction_id());
     } else {
         // 如果日志失败，保留事务以便后续清理
         std::cerr << "Warning: Transaction remains in txn_map due to log failure" << std::endl;
     }
}