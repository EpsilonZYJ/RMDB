/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "lock_manager.h"
#include "../transaction_manager.h"
#include "../txn_defs.h"

/**
 * MVCC 模式下的行级读访问检查
 * 在MVCC中，读操作检查记录的可见性而不是获取锁
 */
bool LockManager::lock_shared_on_record(Transaction *txn, const Rid &rid,
                                        int tab_fd) {
  // 在MVCC模式下进行可见性检查
  if (transaction_manager_) {
    // 检查记录的可见性
    if (!transaction_manager_->IsVisible(rid, txn)) {
      // 记录不可见，快照隔离冲突
      return false;
    }
    return true;
  }

  // 如果没有事务管理器，返回false
  return false;
}

/**
 * MVCC 模式下的行级写访问检查
 * 在MVCC中，写操作需要检查写入冲突
 */
bool LockManager::lock_exclusive_on_record(Transaction *txn, const Rid &rid,
                                           int tab_fd) {
  // 在MVCC模式下检查写入冲突
  if (transaction_manager_) {
    // 获取记录的当前版本链
    auto version_link = transaction_manager_->GetVersionLink(rid);

    if (version_link.has_value()) {
      // 检查写入冲突
      if (version_link->in_progress_) {
        // 情况1：该记录的最新版本属于另一个未提交的事务
        if (version_link->prev_.prev_txn_ != txn->get_transaction_id()) {
          return false; // 写入冲突
        }
      } else {
        // 情况2：该记录的最新版本属于另一个已提交的事务
        // 检查提交时间戳是否晚于当前事务的读时间戳
        try {
          auto undo_log = transaction_manager_->GetUndoLog(version_link->prev_);
          if (undo_log.ts_ > txn->get_start_ts()) {
            return false; // 写入冲突
          }
        } catch (const std::exception &e) {
          // 无法获取undo log，为了安全起见，拒绝写入
          return false;
        }
      }
    }
    return true;
  }

  // 如果没有事务管理器，返回false
  return false;
}

/**
 * MVCC 模式下的表级读访问检查
 */
bool LockManager::lock_shared_on_table(Transaction *txn, int tab_fd) {
  // 在MVCC模式下，表级读锁通常直接允许
  return true;
}

/**
 * MVCC 模式下的表级写访问检查
 */
bool LockManager::lock_exclusive_on_table(Transaction *txn, int tab_fd) {
  // 在MVCC模式下，表级写锁通常直接允许
  return true;
}

/**
 * MVCC 模式下的表级意向读锁检查
 */
bool LockManager::lock_IS_on_table(Transaction *txn, int tab_fd) {
  // 在MVCC模式下，意向锁通常直接允许
  return true;
}

/**
 * MVCC 模式下的表级意向写锁检查
 */
bool LockManager::lock_IX_on_table(Transaction *txn, int tab_fd) {
  // 在MVCC模式下，意向锁通常直接允许
  return true;
}

/**
 * MVCC 模式下的锁释放
 */
bool LockManager::unlock(Transaction *txn, LockDataId lock_data_id) {
  // 在MVCC模式下，不需要显式释放锁
  return true;
}

