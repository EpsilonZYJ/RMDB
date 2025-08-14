/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "lock_manager.h"
/**
 * @description: 申请行级共享锁
 * @return {bool} 加锁是否成功
 * @param {std::shared_ptr<Transaction>} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID 记录所在的表的fd
 * @param {int} tab_fd
 */
bool LockManager::lock_shared_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    LockDataId lock_data_id = LockDataId(tab_fd, rid, LockDataType::RECORD);
    return try_lock(lock_data_id, txn, LockMode::SHARED);
}

/**
 * @description: 申请行级排他锁
 * @return {bool} 加锁是否成功
 * @param {std::shared_ptr<Transaction>} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID
 * @param {int} tab_fd 记录所在的表的fd
 */
bool LockManager::lock_exclusive_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    LockDataId lock_data_id(tab_fd, rid, LockDataType::RECORD);
    return try_lock(lock_data_id, txn, LockMode::EXLUCSIVE);
}

/**
 * @description: 申请表级读锁
 * @return {bool} 返回加锁是否成功
 * @param {std::shared_ptr<Transaction>} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_shared_on_table(Transaction* txn, int tab_fd) {
    LockDataId lock_data_id(tab_fd,LockDataType::TABLE);
    return try_lock(lock_data_id, txn, LockMode::SHARED);
}

/**
 * @description: 申请表级写锁
 * @return {bool} 返回加锁是否成功
 * @param {std::shared_ptr<Transaction>} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_exclusive_on_table(Transaction* txn, int tab_fd) {
    LockDataId lock_data_id(tab_fd,LockDataType::TABLE);
    return try_lock(lock_data_id, txn, LockMode::EXLUCSIVE);
}

/**
 * @description: 申请表级意向读锁
 * @return {bool} 返回加锁是否成功
 * @param {std::shared_ptr<Transaction>} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IS_on_table(Transaction* txn, int tab_fd) {
    LockDataId lock_data_id(tab_fd,LockDataType::TABLE);
    return try_lock(lock_data_id, txn, LockMode::INTENTION_SHARED);
}

/**
 * @description: 申请表级意向写锁
 * @return {bool} 返回加锁是否成功
 * @param {std::shared_ptr<Transaction>} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IX_on_table(Transaction* txn, int tab_fd) {
    LockDataId lock_data_id(tab_fd,LockDataType::TABLE);
    return try_lock(lock_data_id, txn, LockMode::INTENTION_EXCLUSIVE);
}

/**
 * @description: 申请表级共享意向排他锁
 * @return {bool} 返回加锁是否成功
 * @param {std::shared_ptr<Transaction>} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_SIX_on_table(Transaction* txn, int tab_fd) {
    LockDataId lock_data_id(tab_fd,LockDataType::TABLE);
    return try_lock(lock_data_id, txn, LockMode::S_IX);
}
/**
 * @description: 释放锁
 * @return {bool} 返回解锁是否成功
 * @param {std::shared_ptr<Transaction>} txn 要释放锁的事务对象指针
 * @param {LockDataId} lock_data_id 要释放的锁ID
 */
bool LockManager::unlock(Transaction* txn, LockDataId lock_data_id) {
    std::unique_lock<std::mutex> lock(latch_);
    txn->set_state(TransactionState::SHRINKING);
    auto it = lock_table_[lock_data_id].request_queue_.begin();
    while(it != lock_table_[lock_data_id].request_queue_.end()){
        if((*it)->txn_id_ == txn->get_transaction_id()){
            break;
        }
        it++;
    }

    if(it != lock_table_[lock_data_id].request_queue_.end()){
        if(lock_table_[lock_data_id].request_queue_.size() == 1){
            lock_table_[lock_data_id].request_queue_.erase(it);
            lock_table_[lock_data_id].group_lock_mode_ = GroupLockMode::NON_LOCK;
        }else if(lock_table_[lock_data_id].request_queue_.size() > 1){
            lock_table_[lock_data_id].request_queue_.erase(it);
            update_group_lock_mode(lock_data_id);
        }
    }
    lock_table_[lock_data_id].cv_.notify_all();
    return true;
}

/**
 * @description: 尝试加锁
 * @return {bool} 返回加锁是否成功
 * @param {LockDataId} lock_data_id 要加锁的数据ID
 * @param {std::shared_ptr<Transaction>} txn 要加锁的事务对象指针
 * @param {LockMode} lock_mode 加锁模式
 * @note 直到加锁成功，该函数才会返回true。如果不允许加锁会抛出事务回滚异常回滚当前事务。
 */
bool LockManager::try_lock(LockDataId lock_data_id, Transaction* txn, LockMode lock_mode) {
    std::unique_lock<std::mutex> lock(latch_);
    //确认当前是否持有该data_id的锁
    if (txn->get_lock_set()->find(lock_data_id) != txn->get_lock_set()->end()) {
        auto &lock_request_queue = lock_table_[lock_data_id].request_queue_;
        auto lock_request_it = lock_request_queue.begin();
        while(lock_request_it != lock_request_queue.end()) {
            if ((*lock_request_it)->txn_id_ == txn->get_transaction_id()) {
                break;
            }
            lock_request_it++;
        }
        assert(lock_request_it != lock_request_queue.end());
        if((*lock_request_it)->lock_mode_ ==  lock_mode){
            return true;
        }else if((*lock_request_it)->lock_mode_ == LockMode::EXLUCSIVE){
            return true;
        }else if(lock_data_id.type_ == LockDataType::TABLE && (*lock_request_it)->lock_mode_ == LockMode::S_IX && lock_mode != LockMode::EXLUCSIVE){
            return true;
        }
        else{//尝试升级锁。此处先从已有队列中移除，但并未释放锁，下面尝试重新获得锁。
            lock_request_queue.erase(lock_request_it);
            update_group_lock_mode(lock_data_id);
        }
    }
    txn->set_state(TransactionState::GROWING);
    auto new_lock_request = std::make_shared<LockRequest>(txn->get_transaction_id(), lock_mode, txn->get_start_ts());
    //如果当前没有创建过加锁队列
    if(lock_table_.find(lock_data_id) == lock_table_.end()){
        lock_table_[lock_data_id].request_queue_.push_back(new_lock_request);
        txn->get_lock_set()->insert(lock_data_id);
        update_group_lock_mode(lock_data_id);
        return true;
    }else{
        //如果没有等待队列，进行相容性测试，通过即可加锁。
        if(lock_table_[lock_data_id].wait_queue_.empty()){
            if(lock_compatible_check(lock_data_id, txn, lock_mode)){
                lock_table_[lock_data_id].request_queue_.push_back(new_lock_request);
                txn->get_lock_set()->insert(lock_data_id);
                update_group_lock_mode(lock_data_id);
                return true;
            }
        }
        //进行wait-die策略检查
        if(!wait_die_check(lock_data_id, txn, lock_mode)){
            throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
        }
        //将请求加入排队队列
        lock_table_[lock_data_id].wait_queue_.push_back(new_lock_request);
        //开始排队
        while(!wake_up_check(lock_data_id, txn, lock_mode)){//唤醒条件：当前事务的请求位于排队队头,且能通过相容性检查。
            lock_table_[lock_data_id].cv_.wait(lock);
        }
        //唤醒后，将被授予锁
        assert(lock_table_[lock_data_id].wait_queue_.front()->txn_id_ == txn->get_transaction_id());
        lock_table_[lock_data_id].wait_queue_.pop_front();
        lock_table_[lock_data_id].request_queue_.push_back(new_lock_request);
        txn->get_lock_set()->insert(lock_data_id);
        update_group_lock_mode(lock_data_id);
        return true;
    }
}

bool LockManager::lock_compatible_check(LockDataId lock_data_id, Transaction* txn, LockMode lock_mode)
{
    switch(lock_table_[lock_data_id].group_lock_mode_){
        case GroupLockMode::NON_LOCK:
            return true;
        case GroupLockMode::IS:
            if(lock_mode != LockMode::EXLUCSIVE){
                return true;
            }
            break;
        case GroupLockMode::IX:
            if(lock_mode == LockMode::INTENTION_EXCLUSIVE || lock_mode == LockMode::INTENTION_SHARED){
                return true;
            }
            break;
        case GroupLockMode::S:
            if(lock_mode == LockMode::SHARED|| lock_mode == LockMode::INTENTION_SHARED){
                return true;
            }
            break;
        case GroupLockMode::X:
            return false;
        case GroupLockMode::SIX:
            if(lock_mode == LockMode::INTENTION_SHARED){
                return true;
            }
            break;
    }
    return false;
}

bool LockManager::wait_die_check(LockDataId lock_data_id, Transaction* txn, LockMode lock_mode)
{
    auto & lock_request_queue = lock_table_[lock_data_id].request_queue_;
    auto & lock_wait_queue = lock_table_[lock_data_id].wait_queue_;
    timestamp_t ts = txn->get_start_ts();
    for(auto &lock_request : lock_request_queue){
        if(ts >= lock_request->txn_timestamp_){
            return false;
        }
    }
    for(auto &lock_request : lock_wait_queue){
        if(ts >= lock_request->txn_timestamp_){
            return false;
        }
    }
    return true;
}

bool LockManager::wake_up_check(LockDataId lock_data_id, Transaction* txn, LockMode lock_mode)
{
    return txn->get_transaction_id() == lock_table_[lock_data_id].wait_queue_.front()->txn_id_ && lock_compatible_check(lock_data_id, txn, lock_mode);
}

