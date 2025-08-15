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
#include <algorithm>


inline void LockManager::check_wait_die(const std::shared_ptr<LockRequestQueue>& lock_request_queue, Transaction* txn, LockMode requested_mode) {
    // 只检查已授予的锁且与请求锁冲突的事务
    bool should_abort = false;
    GroupLockMode req_group_mode = get_group_lock_mode(requested_mode);
    
    for (const auto& req : lock_request_queue->request_queue_) {
        // 只考虑已授予的锁
        if (req->granted_) {
            // 只检查锁冲突的情况
            if (!lock_compatible(get_group_lock_mode(req->lock_mode_), req_group_mode)) {
                // 只有当持有锁的事务更老（ID更小）时，才需要中止
                if (req->txn_id_ < txn->get_transaction_id()) {
                    should_abort = true;
                    break;
                }
            }
        }
    }
    
    if (should_abort) {
        throw TransactionAbortException(txn->get_transaction_id(), AbortReason::DEADLOCK_PREVENTION);
    }
}

inline std::shared_ptr<LockManager::LockRequestQueue> LockManager::get_lock_request_queue(const LockDataId& lock_data_id)
{
    std::scoped_lock<std::mutex> lock(latch_);
    if(lock_table_.find(lock_data_id) == lock_table_.end()) {
        std::shared_ptr<LockRequestQueue> lock_request_queue = std::make_shared<LockRequestQueue>();
        lock_table_.emplace(lock_data_id, lock_request_queue);
        lock_request_queue->group_lock_mode_ = GroupLockMode::NON_LOCK;
    }
    return lock_table_[lock_data_id];
}

bool LockManager::check_and_execute_lock(std::shared_ptr<LockRequestQueue> lock_request_queue, 
                                         std::shared_ptr<LockRequest> lock_request, 
                                         Transaction* txn, 
                                         GroupLockMode lock_mode) {
    std::unique_lock<std::mutex> queue_lock(lock_request_queue->latch_);
    
    // 检查锁兼容性
    if(!lock_compatible(lock_request_queue->group_lock_mode_, lock_mode)) {
        // 使用修改后的Wait-Die检查
        check_wait_die(lock_request_queue, txn, lock_request->lock_mode_);
        
        // 若通过检查，则等待
        lock_request_queue->request_queue_.emplace_back(lock_request);
        
        // 超时机制保持不变
        auto now = std::chrono::system_clock::now();
        if(!lock_request_queue->cv_.wait_until(queue_lock, 
                                          now + std::chrono::seconds(5), 
                                          [&](){ return lock_request->granted_; })) {
            // 超时处理
            lock_request_queue->request_queue_.remove(lock_request);
            throw TransactionAbortException(txn->get_transaction_id(), AbortReason:: DEADLOCK_PREVENTION);
        }
    } else {
        // 锁兼容，直接授予
        lock_request->granted_ = true;
        lock_request_queue->request_queue_.emplace_back(lock_request);
        
        // 更新组锁模式
        if(lock_mode > lock_request_queue->group_lock_mode_) {
            lock_request_queue->group_lock_mode_ = lock_mode;
        }
    }
    
    return true;
}

/**
 * @description: 申请行级共享锁
 * @return {bool} 加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID 记录所在的表的fd
 * @param {int} tab_fd
 */
bool LockManager::lock_shared_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    if(txn->get_lock_set()->find(LockDataId(tab_fd, rid, LockDataType::RECORD)) != txn->get_lock_set()->end())
        return true;
    lock_IS_on_table(txn, tab_fd);
    const LockDataId lock_data_id = LockDataId(tab_fd, rid, LockDataType::RECORD);
    std::shared_ptr<LockRequestQueue> lock_request_queue = get_lock_request_queue(lock_data_id);
    std::shared_ptr<LockRequest> lock_request = std::make_shared<LockRequest>(txn->get_transaction_id(), LockMode::SHARED);
    
    // 检查是否有事务已经持有排他锁
    // 如果有事务持有排他锁，加入等待队列，等待条件变量唤醒
    check_and_execute_lock(lock_request_queue, lock_request, txn, GroupLockMode::S);
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

/**
 * @description: 申请行级排他锁
 * @return {bool} 加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID
 * @param {int} tab_fd 记录所在的表的fd
 */
bool LockManager::lock_exclusive_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    if(txn->get_lock_set()->find(LockDataId(tab_fd, rid, LockDataType::RECORD)) != txn->get_lock_set()->end())
        return upgrade_lock_on_record(txn, rid, tab_fd);
    lock_IX_on_table(txn, tab_fd);
    const LockDataId lock_data_id = LockDataId(tab_fd, rid, LockDataType::RECORD);
    std::shared_ptr<LockRequestQueue> lock_request_queue = get_lock_request_queue(lock_data_id);
    std::shared_ptr<LockRequest> lock_request = std::make_shared<LockRequest>(txn->get_transaction_id(), LockMode::EXLUCSIVE);
    // 检查是否有事务已经持有锁
    check_and_execute_lock(lock_request_queue, lock_request, txn, GroupLockMode::X);
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

/**
 * @description: 申请表级读锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_shared_on_table(Transaction* txn, int tab_fd) {
    //std::cout << "txn " << txn->get_transaction_id() << " lock_shared_on_table " << tab_fd << std::endl;
    if(txn->get_lock_set()->find(LockDataId(tab_fd, LockDataType::TABLE)) != txn->get_lock_set()->end())
        {
            return upgrade_lock_on_table(txn, tab_fd, LockMode::SHARED);
        }
    const LockDataId lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    std::shared_ptr<LockRequestQueue> lock_request_queue = get_lock_request_queue(lock_data_id);
    std::shared_ptr<LockRequest> lock_request = std::make_shared<LockRequest>(txn->get_transaction_id(), LockMode::SHARED);
    // 检查是否有事务已经持有排他锁
    // 如果有事务持有排他锁，加入等待队列，等待条件变量唤醒
    check_and_execute_lock(lock_request_queue, lock_request, txn, GroupLockMode::S);
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

/**
 * @description: 申请表级写锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_exclusive_on_table(Transaction* txn, int tab_fd) {
    if(txn->get_lock_set()->find(LockDataId(tab_fd, LockDataType::TABLE)) != txn->get_lock_set()->end())
        return upgrade_lock_on_table(txn, tab_fd, LockMode::EXLUCSIVE);
    // std::cout << "txn " << txn->get_transaction_id() << " lock_exclusive_on_table " << tab_fd << std::endl;
    const LockDataId lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    std::shared_ptr<LockRequestQueue> lock_request_queue = get_lock_request_queue(lock_data_id);
    std::shared_ptr<LockRequest> lock_request = std::make_shared<LockRequest>(txn->get_transaction_id(), LockMode::EXLUCSIVE);
    // 检查是否有事务已经持有锁
    check_and_execute_lock(lock_request_queue, lock_request, txn, GroupLockMode::X);
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

bool LockManager::upgrade_lock_on_record(Transaction* txn,const Rid& rid, int tab_fd)
{
    const LockDataId lock_data_id = LockDataId(tab_fd, rid, LockDataType::RECORD);
    std::shared_ptr<LockRequestQueue> lock_request_queue = lock_table_[lock_data_id];
    std::unique_lock<std::mutex> queue_lock(lock_request_queue->latch_);
    std::shared_ptr<LockRequest> lock_request;
    for(auto& tmp : lock_request_queue->request_queue_) {
        if(tmp->txn_id_ == txn->get_transaction_id()) {
            lock_request = tmp;
            break;
        }
    }
    if(lock_request->lock_mode_ == LockMode::EXLUCSIVE)
        return true;
    else
    {
        check_wait_die(lock_request_queue, txn, LockMode::EXLUCSIVE);
    }
    if(lock_request_queue->request_queue_.size() == 1) {
        lock_request_queue->group_lock_mode_ = GroupLockMode::X;
        lock_request->granted_ = true;
        lock_request->lock_mode_ = LockMode::EXLUCSIVE;
    }
    else {
        std::shared_ptr<LockRequest> new_lock_request(new LockRequest(txn->get_transaction_id(), LockMode::EXLUCSIVE));
        lock_request_queue->upgrade_queue_.emplace_back(new_lock_request);
        lock_request_queue->cv_.wait(queue_lock, [&](){
            return new_lock_request->granted_;
        });
        lock_request->lock_mode_ = LockMode::EXLUCSIVE;
        lock_request_queue->upgrade_queue_.remove(new_lock_request);
    }
    return true;
}

bool LockManager::upgrade_lock_on_table(Transaction* txn, int tab_fd, LockMode lock_mode) {
    const LockDataId lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    std::shared_ptr<LockRequestQueue> lock_request_queue = lock_table_[lock_data_id];
    std::unique_lock<std::mutex> queue_lock(lock_request_queue->latch_);
    std::shared_ptr<LockRequest> lock_request;
    for(auto& tmp : lock_request_queue->request_queue_) {
        if(tmp->txn_id_ == txn->get_transaction_id()) {
            lock_request = tmp;
            break;
        }
    }
    if(get_group_lock_mode(lock_request->lock_mode_) >= get_group_lock_mode(lock_mode))
        return true;
    else
    {
        check_wait_die(lock_request_queue, txn, lock_mode);
    }
    if(lock_request_queue->request_queue_.size() == 1 || lock_compatible(lock_request_queue->group_lock_mode_, get_group_lock_mode(lock_mode))) {
        lock_request->lock_mode_ = lock_mode;
        lock_request_queue->group_lock_mode_ = std::max(lock_request_queue->group_lock_mode_, get_group_lock_mode(lock_request->lock_mode_));
        lock_request->granted_ = true;
    }
    else {
        std::shared_ptr<LockRequest> new_lock_request(new LockRequest(txn->get_transaction_id(), lock_mode));
        lock_request_queue->upgrade_queue_.emplace_back(new_lock_request);
        lock_request_queue->cv_.wait(queue_lock, [&](){
            return new_lock_request->granted_;
        });
        lock_request->lock_mode_ = lock_mode;
        lock_request_queue->upgrade_queue_.remove(new_lock_request);
    }
    return true;
}

/**
 * @description: 申请表级意向读锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IS_on_table(Transaction* txn, int tab_fd) {
    if(txn->get_lock_set()->find(LockDataId(tab_fd, LockDataType::TABLE)) != txn->get_lock_set()->end())
        return upgrade_lock_on_table(txn, tab_fd, LockMode::INTENTION_SHARED);
    const LockDataId lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    std::shared_ptr<LockRequestQueue> lock_request_queue = get_lock_request_queue(lock_data_id);
    std::shared_ptr<LockRequest> lock_request = std::make_shared<LockRequest>(txn->get_transaction_id(), LockMode::INTENTION_SHARED);
    // 检查是否有事务已经持有排他锁
    // 如果有事务持有排他锁，加入等待队列，等待条件变量唤醒
    check_and_execute_lock(lock_request_queue, lock_request, txn, GroupLockMode::IS);
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

/**
 * @description: 申请表级意向写锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IX_on_table(Transaction* txn, int tab_fd) {
    if(txn->get_lock_set()->find(LockDataId(tab_fd, LockDataType::TABLE)) != txn->get_lock_set()->end())
        return upgrade_lock_on_table(txn, tab_fd, LockMode::INTENTION_EXCLUSIVE);
    const LockDataId lock_data_id = LockDataId(tab_fd, LockDataType::TABLE);
    std::shared_ptr<LockRequestQueue> lock_request_queue = get_lock_request_queue(lock_data_id);
    std::shared_ptr<LockRequest> lock_request = std::make_shared<LockRequest>(txn->get_transaction_id(), LockMode::INTENTION_EXCLUSIVE);
    // 检查是否有事务已经持有排他锁
    // 如果有事务持有排他锁，加入等待队列，等待条件变量唤醒
    check_and_execute_lock(lock_request_queue, lock_request, txn, GroupLockMode::IX);
    txn->get_lock_set()->insert(lock_data_id);
    return true;
}

LockManager::GroupLockMode LockManager::get_group_lock_mode(LockMode lock_mode)
{
    switch(lock_mode) {
        case LockMode::SHARED:
            return GroupLockMode::S;
        case LockMode::EXLUCSIVE:
            return GroupLockMode::X;
        case LockMode::INTENTION_SHARED:
            return GroupLockMode::IS;
        case LockMode::INTENTION_EXCLUSIVE:
            return GroupLockMode::IX;
        case LockMode::S_IX:
            return GroupLockMode::SIX;
        default:
            return GroupLockMode::NON_LOCK;
    }

}

/**
 * @description: 释放锁
 * @return {bool} 返回解锁是否成功
 * @param {Transaction*} txn 要释放锁的事务对象指针
 * @param {LockDataId} lock_data_id 要释放的锁ID
 */
bool LockManager::unlock(Transaction* txn, LockDataId lock_data_id) {
    std::shared_ptr<LockRequestQueue> lock_request_queue;
    {
        std::scoped_lock<std::mutex> lock(latch_);
        if(lock_table_.find(lock_data_id) == lock_table_.end()) {
            return false;
        }
        lock_request_queue = lock_table_[lock_data_id];
    }
    std::unique_lock<std::mutex> queue_lock(lock_request_queue->latch_);
    auto& request_queue = lock_request_queue->request_queue_;
    for(auto it = request_queue.begin(); it != request_queue.end();) {
        if((*it)->txn_id_ == txn->get_transaction_id()) {
            it = request_queue.erase(it);
            // 删除break语句，继续检查所有请求
        } else {
            ++it;
        }
    }
    txn->get_lock_set()->erase(lock_data_id);
    // 重新计算当前锁模式
    GroupLockMode new_mode = GroupLockMode::NON_LOCK;
    for(auto& req : request_queue) {
        if(req->granted_) {
            GroupLockMode mode = get_group_lock_mode(req->lock_mode_);
            if(mode > new_mode) new_mode = mode;
        }
    }
    lock_request_queue->group_lock_mode_ = new_mode;
    
    if (!lock_request_queue->upgrade_queue_.empty()) {
        auto upgrade_req = lock_request_queue->upgrade_queue_.front();
        GroupLockMode req_mode = get_group_lock_mode(upgrade_req->lock_mode_);
        
        // 检查是否可以授予升级请求
        if (lock_compatible(new_mode, req_mode)) {
            // 找到对应的原始锁请求并更新
            for (auto& req : request_queue) {
                if (req->txn_id_ == upgrade_req->txn_id_) {
                    req->lock_mode_ = upgrade_req->lock_mode_;
                    req->granted_ = true;
                    
                    // 更新组锁模式
                    if (req_mode > new_mode) {
                        lock_request_queue->group_lock_mode_ = req_mode;
                    }
                    break;
                }
            }
            
            upgrade_req->granted_ = true;
            lock_request_queue->upgrade_queue_.pop_front();
        }
    }

    // 尝试授予等待的锁
    bool granted_new = false;
    for(auto& req : request_queue) {
        if(!req->granted_) {
            GroupLockMode req_mode = get_group_lock_mode(req->lock_mode_);
            if(lock_compatible(new_mode, req_mode)) {
                req->granted_ = true;
                granted_new = true;
                
                // 更新组锁模式
                if(req_mode > new_mode) {
                    new_mode = req_mode;
                    lock_request_queue->group_lock_mode_ = new_mode;
                }
            }
        }
    }
    
    // 无论是否授予新锁，都通知所有等待线程重新检查
    lock_request_queue->cv_.notify_all();
    
    return true;
}