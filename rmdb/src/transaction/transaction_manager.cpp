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

        // [MVCC 新增代码] - 设置读时间戳和事务开始标记
        if (concurrency_mode_ == ConcurrencyMode::MVCC) {
            std::unique_lock<std::shared_mutex> lock(txn_map_mutex_);
            txn->set_start_ts(GetWatermark());
            std::cout << "事务 #" << txn_id << " 开始，读时间戳：" << txn->get_start_ts() << std::endl;
        }
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
    
    //事务结束，清理所有写记录

    // [MVCC 新增代码] - 设置提交时间戳
    if (concurrency_mode_ == ConcurrencyMode::MVCC) {
        timestamp_t commit_ts = GetNextTimestamp();
        txn->commit_ts_ = commit_ts;
        std::cout << "事务 #" << txn->get_transaction_id() << " 提交，提交时间戳：" << commit_ts << std::endl;
    }

    auto write_set = txn->get_write_set();
    while (!write_set->empty()) {
        WriteRecord* record = write_set->front();

        // [MVCC 新增代码] - 更新版本链
        if (concurrency_mode_ == ConcurrencyMode::MVCC) {
            // 更新版本链中的时间戳信息
            UpdateVersionTimestamp(record->GetRid(), commit_ts);
        }

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
    
    // 更新事务状态为已提交
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
                 fh->delete_record(record->GetRid(), nullptr);
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
     
     // 清空锁集合
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
     
     // 更新事务状态为已回滚
     txn->set_state(TransactionState::ABORTED);
     
     // 从事务表中移除事务
     if (log_success) {
         std::unique_lock<std::mutex> lock(latch_);
         txn_map.erase(txn->get_transaction_id());
     } else {
         // 如果日志失败，保留事务以便后续清理
         std::cerr << "Warning: Transaction remains in txn_map due to log failure" << std::endl;
     }
}


// [MVCC 新增方法]
timestamp_t TransactionManager::GetNextTimestamp() {
    return ++current_timestamp_;
}

/**
 * @brief 更新一个撤销链接，该链接将表堆元组与第一个撤销日志连接起来
 * @param rid 记录ID
 * @param prev_link 新的撤销链接
 * @param check 可选的检查函数
 * @return 是否更新成功
 */
bool TransactionManager::UpdateUndoLink(Rid rid, std::optional<UndoLink> prev_link,
                    std::function<bool(std::optional<UndoLink>)> &&check) {
    // 获取PageVersionInfo对象
    auto page_id = rid.page_no;
    std::shared_ptr<PageVersionInfo> page_version_info;
    
    {
        std::shared_lock<std::shared_mutex> lock(version_info_mutex_);
        auto it = version_info_.find(page_id);
        if (it == version_info_.end()) {
            // 如果页面不存在，需要创建
            lock.unlock();
            std::unique_lock<std::shared_mutex> u_lock(version_info_mutex_);
            // 双重检查模式，避免竞争条件
            it = version_info_.find(page_id);
            if (it == version_info_.end()) {
                page_version_info = std::make_shared<PageVersionInfo>();
                version_info_[page_id] = page_version_info;
            } else {
                page_version_info = it->second;
            }
        } else {
            page_version_info = it->second;
        }
    }
    
    // 获取槽的版本信息
    std::unique_lock<std::shared_mutex> lock(page_version_info->mutex_);
    auto slot_no = rid.slot_no;
    auto it = page_version_info->prev_version_.find(slot_no);
    
    // 如果指定了检查函数，则执行检查
    if (check) {
        std::optional<UndoLink> current_link;
        if (it != page_version_info->prev_version_.end()) {
            current_link = it->second.prev_;
        }
        
        if (!check(current_link)) {
            return false;
        }
    }
    
    // 更新或创建撤销链接
    if (prev_link.has_value()) {
        if (it == page_version_info->prev_version_.end()) {
            // 创建新的版本撤销链接
            VersionUndoLink version_link;
            version_link.prev_ = *prev_link;
            page_version_info->prev_version_[slot_no] = version_link;
        } else {
            // 更新现有链接
            it->second.prev_ = *prev_link;
        }
    } else if (it != page_version_info->prev_version_.end()) {
        // 如果不提供新链接且存在旧链接，则移除
        page_version_info->prev_version_.erase(slot_no);
    }
    
    return true;
}

/**
 * @brief 更新版本链接
 * @param rid 记录ID
 * @param prev_version 新的版本链接
 * @param check 可选的检查函数
 * @return 是否更新成功
 */
bool TransactionManager::UpdateVersionLink(Rid rid, std::optional<VersionUndoLink> prev_version,
                       std::function<bool(std::optional<VersionUndoLink>)> &&check) {
    // 获取PageVersionInfo对象
    auto page_id = rid.page_no;
    std::shared_ptr<PageVersionInfo> page_version_info;
    
    {
        std::shared_lock<std::shared_mutex> lock(version_info_mutex_);
        auto it = version_info_.find(page_id);
        if (it == version_info_.end()) {
            // 如果页面不存在，需要创建
            lock.unlock();
            std::unique_lock<std::shared_mutex> u_lock(version_info_mutex_);
            // 双重检查模式
            it = version_info_.find(page_id);
            if (it == version_info_.end()) {
                page_version_info = std::make_shared<PageVersionInfo>();
                version_info_[page_id] = page_version_info;
            } else {
                page_version_info = it->second;
            }
        } else {
            page_version_info = it->second;
        }
    }
    
    // 获取槽的版本信息并更新
    std::unique_lock<std::shared_mutex> lock(page_version_info->mutex_);
    auto slot_no = rid.slot_no;
    
    // 执行检查
    if (check) {
        std::optional<VersionUndoLink> current_version;
        auto it = page_version_info->prev_version_.find(slot_no);
        if (it != page_version_info->prev_version_.end()) {
            current_version = it->second;
        }
        
        if (!check(current_version)) {
            return false;
        }
    }
    
    // 更新或创建版本链接
    if (prev_version.has_value()) {
        page_version_info->prev_version_[slot_no] = *prev_version;
    } else {
        page_version_info->prev_version_.erase(slot_no);
    }
    
    return true;
}

/**
 * @brief 获取表堆元组的第一个撤销日志
 * @param rid 记录ID
 * @return 可选的UndoLink
 */
std::optional<UndoLink> TransactionManager::GetUndoLink(Rid rid) {
    auto page_id = rid.page_no;
    
    std::shared_lock<std::shared_mutex> lock(version_info_mutex_);
    auto it = version_info_.find(page_id);
    if (it == version_info_.end()) {
        return std::nullopt;  // 页面不存在
    }
    
    auto page_version_info = it->second;
    std::shared_lock<std::shared_mutex> page_lock(page_version_info->mutex_);
    
    auto slot_it = page_version_info->prev_version_.find(rid.slot_no);
    if (slot_it == page_version_info->prev_version_.end()) {
        return std::nullopt;  // 槽没有版本信息
    }
    
    return slot_it->second.prev_;
}

/**
 * @brief 获取表堆元组的版本链接
 * @param rid 记录ID
 * @return 可选的VersionUndoLink
 */
std::optional<VersionUndoLink> TransactionManager::GetVersionLink(Rid rid) {
    auto page_id = rid.page_no;
    
    std::shared_lock<std::shared_mutex> lock(version_info_mutex_);
    auto it = version_info_.find(page_id);
    if (it == version_info_.end()) {
        return std::nullopt;  // 页面不存在
    }
    
    auto page_version_info = it->second;
    std::shared_lock<std::shared_mutex> page_lock(page_version_info->mutex_);
    
    auto slot_it = page_version_info->prev_version_.find(rid.slot_no);
    if (slot_it == page_version_info->prev_version_.end()) {
        return std::nullopt;  // 槽没有版本信息
    }
    
    return slot_it->second;
}

/**
 * @brief 访问事务撤销日志缓冲区并获取撤销日志
 * @param link 撤销链接
 * @return 可选的撤销日志
 */
std::optional<UndoLog> TransactionManager::GetUndoLogOptional(UndoLink link) {
    // 检查事务有效性
    std::shared_lock<std::mutex> lock(latch_);
    auto txn_it = txn_map.find(link.txn_id_);
    if (txn_it == txn_map.end()) {
        return std::nullopt;  // 事务不存在
    }
    
    Transaction* txn = txn_it->second;
    lock.unlock();
    
    // 获取事务的撤销日志
    try {
        return txn->GetUndoLog(link.log_idx_);
    } catch (const std::exception& e) {
        // 索引超出范围或其他错误
        return std::nullopt;
    }
}

/**
 * @brief 访问事务撤销日志缓冲区并获取撤销日志
 * @param link 撤销链接
 * @return 撤销日志
 */
UndoLog TransactionManager::GetUndoLog(UndoLink link) {
    // 首先尝试获取可选的撤销日志
    auto undo_log_opt = GetUndoLogOptional(link);
    if (!undo_log_opt.has_value()) {
        throw TransactionAbortException(
            AbortReason::ATTEMPTED_OPERATION_ON_ABORTED_TRANSACTION,
            "尝试访问不存在的事务或无效的撤销日志索引"
        );
    }
    
    return *undo_log_opt;
}

/**
 * @brief 获取系统中的最低读时间戳
 * @return 最低读时间戳
 */
timestamp_t TransactionManager::GetWatermark() {
    // 获取当前所有活跃事务的最小读时间戳
    timestamp_t min_ts = next_timestamp_.load();
    
    std::shared_lock<std::shared_mutex> lock(txn_map_mutex_);
    for (const auto& pair : txn_map) {
        Transaction* txn = pair.second;
        if (txn->get_state() == TransactionState::GROWING) {
            min_ts = std::min(min_ts, txn->get_start_ts());
        }
    }
    
    // 如果没有活跃事务，返回最后提交的时间戳
    if (min_ts == next_timestamp_.load()) {
        return last_commit_ts_.load();
    }
    
    return min_ts;
}

/**
 * @brief 垃圾回收，清理不再需要的版本
 */
void TransactionManager::GarbageCollection() {
    // 获取当前水位线（最小读时间戳）
    timestamp_t watermark = GetWatermark();
    
    // 清理过期的版本信息
    std::vector<page_id_t> empty_pages;
    {
        std::shared_lock<std::shared_mutex> lock(version_info_mutex_);
        for (auto& pair : version_info_) {
            page_id_t page_id = pair.first;
            auto page_version_info = pair.second;
            
            std::unique_lock<std::shared_mutex> page_lock(page_version_info->mutex_);
            std::vector<slot_offset_t> slots_to_remove;
            
            // 找出可以删除的槽
            for (auto& slot_pair : page_version_info->prev_version_) {
                // 检查版本链是否仍被使用
                // 如果版本的时间戳小于水位线，说明没有活跃事务能看到它
                if (slot_pair.second.in_progress_ == false) {
                    // 获取该版本链上的所有事务时间戳
                    bool can_remove = true;
                    UndoLink current_link = slot_pair.second.prev_;
                    
                    while (current_link.txn_id_ != INVALID_TXN_ID) {
                        auto undo_log_opt = GetUndoLogOptional(current_link);
                        if (!undo_log_opt.has_value()) {
                            break;  // 日志不存在，可能已经被回收
                        }
                        
                        const UndoLog& log = *undo_log_opt;
                        if (log.ts_ >= watermark) {
                            // 有活跃事务可能看到此版本
                            can_remove = false;
                            break;
                        }
                        
                        // 继续检查前一个版本
                        if (!log.prev_.has_value()) {
                            break;
                        }
                        current_link = *log.prev_;
                    }
                    
                    if (can_remove) {
                        slots_to_remove.push_back(slot_pair.first);
                    }
                }
            }
            
            // 删除不再需要的槽
            for (auto slot : slots_to_remove) {
                page_version_info->prev_version_.erase(slot);
            }
            
            // 如果页面没有版本信息，标记为可删除
            if (page_version_info->prev_version_.empty()) {
                empty_pages.push_back(page_id);
            }
        }
    }
    
    // 删除空页面
    if (!empty_pages.empty()) {
        std::unique_lock<std::shared_mutex> lock(version_info_mutex_);
        for (auto page_id : empty_pages) {
            auto it = version_info_.find(page_id);
            if (it != version_info_.end() && it->second->prev_version_.empty()) {
                version_info_.erase(page_id);
            }
        }
        
        std::cout << "垃圾回收：清理了 " << empty_pages.size() << " 个空页面" << std::endl;
    }
}

/**
 * @brief 更新版本的时间戳
 * @param rid 记录ID
 * @param commit_ts 提交时间戳
 */
void TransactionManager::UpdateVersionTimestamp(Rid rid, timestamp_t commit_ts) {
    auto page_id = rid.page_no;
    
    std::shared_lock<std::shared_mutex> lock(version_info_mutex_);
    auto it = version_info_.find(page_id);
    if (it == version_info_.end()) {
        return;  // 页面不存在
    }
    
    auto page_version_info = it->second;
    std::unique_lock<std::shared_mutex> page_lock(page_version_info->mutex_);
    
    auto slot_it = page_version_info->prev_version_.find(rid.slot_no);
    if (slot_it != page_version_info->prev_version_.end()) {
        // 将in_progress标志设置为false，表示事务已提交
        slot_it->second.in_progress_ = false;
        
        // 更新最后提交的时间戳
        last_commit_ts_.store(commit_ts);
    }
}

/**
 * @brief 检查写冲突
 * @param txn 当前事务
 * @param rid 记录ID
 * @return true表示存在冲突，false表示无冲突
 */
bool TransactionManager::CheckWriteConflict(Transaction* txn, const Rid& rid) {
    if (concurrency_mode_ != ConcurrencyMode::MVCC) {
        return false;  // 非MVCC模式下不检查写冲突
    }
    
    auto version_link_opt = GetVersionLink(rid);
    if (!version_link_opt.has_value()) {
        // 记录没有版本链，没有冲突
        return false;
    }
    
    const VersionUndoLink& link = *version_link_opt;
    
    // 冲突检测条件1：记录被另一个未提交的事务持有
    if (link.prev_.txn_id_ != INVALID_TXN_ID && link.prev_.txn_id_ != txn->get_transaction_id()) {
        Transaction* owner_txn = get_transaction(link.prev_.txn_id_);
        if (owner_txn != nullptr && owner_txn->get_state() != TransactionState::COMMITTED) {
            std::cout << "写冲突：事务 #" << txn->get_transaction_id() 
                     << " 尝试更新记录，但该记录已被事务 #" << link.prev_.txn_id_ << " 持有(未提交)" << std::endl;
            return true;
        }
    }
    
    // 获取记录的最后一个修改时间戳
    timestamp_t record_ts = 0;
    auto undo_log_opt = GetUndoLogOptional(link.prev_);
    if (undo_log_opt.has_value()) {
        record_ts = undo_log_opt->ts_;
    }
    
    // 冲突检测条件2：记录的最后修改时间戳大于当前事务的读时间戳
    if (record_ts > txn->get_start_ts()) {
        std::cout << "写冲突：事务 #" << txn->get_transaction_id() 
                 << " 尝试更新记录，但该记录的修改时间戳(" << record_ts 
                 << ")大于当前事务读时间戳(" << txn->get_start_ts() << ")" << std::endl;
        return true;
    }
    
    return false;
}