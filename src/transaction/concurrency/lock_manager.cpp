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

bool LockManager::lock_IS_on_table_internal(Transaction* txn, int tab_fd) {
    if (txn == nullptr) return false;
    
    // 注意：这里不获取latch_，调用者负责锁管理
    
    LockDataId lock_data_id(tab_fd, LockDataType::TABLE);
    auto lock_set = txn->get_lock_set();
    if (lock_set->find(lock_data_id) != lock_set->end()) {
        return true; // 已经持有锁
    }
    
    auto& queue = lock_table_[lock_data_id];
    
    if (can_grant_lock(queue, LockMode::INTENTION_SHARED, txn->get_transaction_id())) {
        LockRequest request(txn->get_transaction_id(), LockMode::INTENTION_SHARED);
        request.granted_ = true;
        queue.request_queue_.push_back(request);
        update_group_lock_mode(queue);
        lock_set->insert(lock_data_id);
        return true;
    }
    
    return false;
}

/**
 * 内部方法：不获取全局锁的表级IX锁申请
 */
bool LockManager::lock_IX_on_table_internal(Transaction* txn, int tab_fd) {
    if (txn == nullptr) return false;
    
    LockDataId lock_data_id(tab_fd, LockDataType::TABLE);
    auto lock_set = txn->get_lock_set();
    if (lock_set->find(lock_data_id) != lock_set->end()) {
        return true;
    }
    
    auto& queue = lock_table_[lock_data_id];
    
    if (can_grant_lock(queue, LockMode::INTENTION_EXCLUSIVE, txn->get_transaction_id())) {
        LockRequest request(txn->get_transaction_id(), LockMode::INTENTION_EXCLUSIVE);
        request.granted_ = true;
        queue.request_queue_.push_back(request);
        update_group_lock_mode(queue);
        lock_set->insert(lock_data_id);
        return true;
    }
    
    return false;
}

/**
 * @description: 检查两种锁模式是否兼容
 * @param {LockMode} mode1 锁模式1
 * @param {LockMode} mode2 锁模式2
 * @return {bool} 是否兼容
 */
bool LockManager::is_compatible(LockMode mode1, LockMode mode2) {
    // 锁兼容性矩阵
    //        S    X    IS   IX   SIX
    // S      ✓    ✗    ✓    ✗    ✗
    // X      ✗    ✗    ✗    ✗    ✗
    // IS     ✓    ✗    ✓    ✓    ✓
    // IX     ✗    ✗    ✓    ✓    ✗
    // SIX    ✗    ✗    ✓    ✗    ✗
    
    if (mode1 == LockMode::SHARED) {
        return mode2 == LockMode::SHARED || mode2 == LockMode::INTENTION_SHARED;
    }
    if (mode1 == LockMode::EXLUCSIVE) {
        return false; // 排他锁与任何锁都不兼容
    }
    if (mode1 == LockMode::INTENTION_SHARED) {
        return mode2 != LockMode::EXLUCSIVE;
    }
    if (mode1 == LockMode::INTENTION_EXCLUSIVE) {
        return mode2 == LockMode::INTENTION_SHARED || mode2 == LockMode::INTENTION_EXCLUSIVE;
    }
    if (mode1 == LockMode::S_IX) {
        return mode2 == LockMode::INTENTION_SHARED;
    }
    return false;
}

/**
 * @description: 检查事务是否可以获得锁
 * @param {LockRequestQueue&} queue 锁请求队列
 * @param {LockMode} mode 请求的锁模式
 * @param {txn_id_t} txn_id 事务ID
 * @return {bool} 是否可以获得锁
 */
bool LockManager::can_grant_lock(const LockRequestQueue& queue, LockMode mode, txn_id_t txn_id) {
    // 检查是否与已授予的锁兼容
    for (const auto& request : queue.request_queue_) {
        if (request.granted_ && request.txn_id_ != txn_id) {
            if (!is_compatible(mode, request.lock_mode_)) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @description: 更新队列的组锁模式
 * @param {LockRequestQueue&} queue 锁请求队列
 */
void LockManager::update_group_lock_mode(LockRequestQueue& queue) {
    GroupLockMode new_mode = GroupLockMode::NON_LOCK;
    
    for (const auto& request : queue.request_queue_) {
        if (request.granted_) {
            GroupLockMode current_mode = GroupLockMode::NON_LOCK;
            
            switch (request.lock_mode_) {
                case LockMode::SHARED:
                    current_mode = GroupLockMode::S;
                    break;
                case LockMode::EXLUCSIVE:
                    current_mode = GroupLockMode::X;
                    break;
                case LockMode::INTENTION_SHARED:
                    current_mode = GroupLockMode::IS;
                    break;
                case LockMode::INTENTION_EXCLUSIVE:
                    current_mode = GroupLockMode::IX;
                    break;
                case LockMode::S_IX:
                    current_mode = GroupLockMode::SIX;
                    break;
            }
            
            // 选择更强的锁模式
            if (static_cast<int>(current_mode) > static_cast<int>(new_mode)) {
                new_mode = current_mode;
            }
        }
    }
    
    queue.group_lock_mode_ = new_mode;
}

/**
 * @description: 申请行级共享锁
 * @return {bool} 加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID 记录所在的表的fd
 * @param {int} tab_fd
 */
bool LockManager::lock_shared_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    if (txn == nullptr) return false;
    
    std::unique_lock<std::mutex> lock(latch_);  // 只获取一次全局锁
    
    // 使用内部方法，避免重复获取锁
    if (!lock_IS_on_table_internal(txn, tab_fd)) {
        return false;
    }
    
    LockDataId lock_data_id(tab_fd, rid, LockDataType::RECORD);
    auto lock_set = txn->get_lock_set();
    if (lock_set->find(lock_data_id) != lock_set->end()) {
        return true;
    }
    
    auto& queue = lock_table_[lock_data_id];
    
    if (can_grant_lock(queue, LockMode::SHARED, txn->get_transaction_id())) {
        LockRequest request(txn->get_transaction_id(), LockMode::SHARED);
        request.granted_ = true;
        queue.request_queue_.push_back(request);
        update_group_lock_mode(queue);
        lock_set->insert(lock_data_id);
        return true;
    }
    
    return false;
}

/**
 * @description: 申请行级排他锁
 * @return {bool} 加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {Rid&} rid 加锁的目标记录ID
 * @param {int} tab_fd 记录所在的表的fd
 */
bool LockManager::lock_exclusive_on_record(Transaction* txn, const Rid& rid, int tab_fd) {
    if (txn == nullptr) return false;
    
    std::unique_lock<std::mutex> lock(latch_);  // 只获取一次全局锁
    
    // 使用内部方法，避免重复获取锁
    if (!lock_IX_on_table_internal(txn, tab_fd)) {
        return false;
    }
    
    LockDataId lock_data_id(tab_fd, rid, LockDataType::RECORD);
    auto lock_set = txn->get_lock_set();
    if (lock_set->find(lock_data_id) != lock_set->end()) {
        return true;
    }
    
    auto& queue = lock_table_[lock_data_id];
    
    if (can_grant_lock(queue, LockMode::EXLUCSIVE, txn->get_transaction_id())) {
        LockRequest request(txn->get_transaction_id(), LockMode::EXLUCSIVE);
        request.granted_ = true;
        queue.request_queue_.push_back(request);
        update_group_lock_mode(queue);
        lock_set->insert(lock_data_id);
        return true;
    }
    
    return false;
}

/**
 * @description: 申请表级读锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_shared_on_table(Transaction* txn, int tab_fd) {
    if (txn == nullptr) return false;
    
    std::unique_lock<std::mutex> lock(latch_);
    
    // 构造表锁ID
    LockDataId lock_data_id(tab_fd, LockDataType::TABLE);
    
    // 检查事务是否已经持有该表的锁
    auto lock_set = txn->get_lock_set();
    if (lock_set->find(lock_data_id) != lock_set->end()) {
        return true; // 已经持有锁
    }
    
    // 获取或创建锁请求队列
    auto& queue = lock_table_[lock_data_id];
    
    // 检查是否可以立即获得锁
    if (can_grant_lock(queue, LockMode::SHARED, txn->get_transaction_id())) {
        // 创建锁请求并授予
        LockRequest request(txn->get_transaction_id(), LockMode::SHARED);
        request.granted_ = true;
        queue.request_queue_.push_back(request);
        
        // 更新组锁模式
        update_group_lock_mode(queue);
        
        // 将锁添加到事务的锁集合中
        lock_set->insert(lock_data_id);
        
        return true;
    }
    
    return false;
}

/**
 * @description: 申请表级写锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
 bool LockManager::lock_exclusive_on_table(Transaction* txn, int tab_fd) {
    if (txn == nullptr) {
        return false;
    }
    std::unique_lock<std::mutex> lock(latch_);
    
    // 构造表锁ID
    LockDataId lock_data_id(tab_fd, LockDataType::TABLE);
    
    // 检查事务是否已经持有该表的锁
    auto lock_set = txn->get_lock_set();
    if (lock_set == nullptr) {
        return false;
    }
    
    if (lock_set->find(lock_data_id) != lock_set->end()) {
        return true; // 已经持有锁
    }
    // 获取或创建锁请求队列
    auto& queue = lock_table_[lock_data_id];
    // 检查是否可以立即获得锁
    if (can_grant_lock(queue, LockMode::EXLUCSIVE, txn->get_transaction_id())) {
        // 创建锁请求并授予
        LockRequest request(txn->get_transaction_id(), LockMode::EXLUCSIVE);
        request.granted_ = true;
        queue.request_queue_.push_back(request);
        update_group_lock_mode(queue);
        // 将锁添加到事务的锁集合中
        lock_set->insert(lock_data_id);
        return true;
    }
    return false;
}

/**
 * @description: 申请表级意向读锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IS_on_table(Transaction* txn, int tab_fd) {
    if (txn == nullptr) return false;
    
    std::unique_lock<std::mutex> lock(latch_);
    return lock_IS_on_table_internal(txn, tab_fd);
}

/**
 * @description: 申请表级意向写锁
 * @return {bool} 返回加锁是否成功
 * @param {Transaction*} txn 要申请锁的事务对象指针
 * @param {int} tab_fd 目标表的fd
 */
bool LockManager::lock_IX_on_table(Transaction* txn, int tab_fd) {
    if (txn == nullptr) return false;
    
    std::unique_lock<std::mutex> lock(latch_);
    return lock_IX_on_table_internal(txn, tab_fd);
}

/**
 * @description: 释放锁
 * @return {bool} 返回解锁是否成功
 * @param {Transaction*} txn 要释放锁的事务对象指针
 * @param {LockDataId} lock_data_id 要释放的锁ID
 */
bool LockManager::unlock(Transaction* txn, LockDataId lock_data_id) {
    if (txn == nullptr) return false;
    std::unique_lock<std::mutex> lock(latch_);
    // 检查锁表中是否存在该锁
    auto it = lock_table_.find(lock_data_id);
    if (it == lock_table_.end()) {
        return false; // 锁不存在
    }
    
    auto& queue = it->second;
    txn_id_t txn_id = txn->get_transaction_id();
    
    // 在队列中查找并移除该事务的锁请求
    bool found = false;
    for (auto req_it = queue.request_queue_.begin(); req_it != queue.request_queue_.end(); ++req_it) {
        if (req_it->txn_id_ == txn_id && req_it->granted_) {
            queue.request_queue_.erase(req_it);
            found = true;
            break;
        }
    } 
    if (!found) {
        return false; // 事务没有持有该锁
    }
    // 从事务的锁集合中移除
    auto lock_set = txn->get_lock_set();
    lock_set->erase(lock_data_id);
    // 更新组锁模式
    update_group_lock_mode(queue);
    // 如果队列为空，从锁表中移除
    if (queue.request_queue_.empty()) {
        lock_table_.erase(it);
    }
    
    return true;
}