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
    txn_map[txn->get_transaction_id()] = txn;
    
    // 写入BEGIN日志记录
    BeginLogRecord log_record(txn->get_transaction_id());
    log_record.prev_lsn_ = txn->get_prev_lsn();
    lsn_t lsn = log_manager->add_log_to_buffer(&log_record);
    txn->set_prev_lsn(lsn);
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
    CommitLogRecord log_record(txn->get_transaction_id());
    log_record.prev_lsn_ = txn->get_prev_lsn();
    lsn_t lsn = log_manager->add_log_to_buffer(&log_record);
    txn->set_prev_lsn(lsn);
    log_manager->flush_log_to_disk();
    
    // 更新事务状态为已提交
    txn->set_state(TransactionState::COMMITTED);
    
    // 从事务表中移除
    txn_map.erase(txn->get_transaction_id());
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
    for (auto &lock : *txn->get_lock_set())
    {
        lock_manager_->unlock(txn, lock);
    }
    txn->get_lock_set()->clear();
    txn->set_state(TransactionState::ABORTED);
    txn_map.erase(txn->get_transaction_id());
}

/** @brief 更新一个撤销链接，该链接将表堆元组与第一个撤销日志连接起来。
 * 在更新之前，将调用 `check` 函数以确保有效性。
 */
bool TransactionManager::UpdateVersionLink(
    Rid rid, std::optional<VersionUndoLink> prev_version,
    std::function<bool(std::optional<VersionUndoLink>)> &&check) {
  
  // 获取页面版本信息
  std::shared_lock<std::shared_mutex> read_lock(version_info_mutex_);
  auto it = version_info_.find(rid.page_no);
  if (it == version_info_.end()) {
    // 页面不存在，需要创建
    read_lock.unlock();
    std::unique_lock<std::shared_mutex> write_lock(version_info_mutex_);
    // 再次检查，避免在锁升级期间其他线程已经创建
    it = version_info_.find(rid.page_no);
    if (it == version_info_.end()) {
      // 创建新的页面版本信息
      auto page_version_info = std::make_shared<PageVersionInfo>();
      version_info_[rid.page_no] = page_version_info;
      it = version_info_.find(rid.page_no);
    }
    write_lock.unlock();
    read_lock.lock();
  }

  auto page_version_info = it->second;
  std::unique_lock<std::shared_mutex> page_lock(page_version_info->mutex_);

  // 查找现有版本链
  auto slot_it = page_version_info->prev_version_.find(rid.slot_no);
  std::optional<VersionUndoLink> current_version = std::nullopt;
  if (slot_it != page_version_info->prev_version_.end()) {
    current_version = slot_it->second;
  }

  // 调用检查函数
  if (check && !check(current_version)) {
    return false;
  }

  // 更新版本链
  if (prev_version.has_value()) {
    page_version_info->prev_version_[rid.slot_no] = *prev_version;
  } else {
    page_version_info->prev_version_.erase(rid.slot_no);
  }

  return true;
}

/** @brief 获取给定记录的版本链接 */
std::optional<VersionUndoLink> TransactionManager::GetVersionLink(Rid rid) {
  std::shared_lock<std::shared_mutex> read_lock(version_info_mutex_);
  auto page_it = version_info_.find(rid.page_no);
  if (page_it == version_info_.end()) {
    return std::nullopt; // 页面不存在
  }
  auto page_version_info = page_it->second;
  std::shared_lock<std::shared_mutex> page_lock(page_version_info->mutex_);

  // 查找槽位
  auto slot_it = page_version_info->prev_version_.find(rid.slot_no);
  if (slot_it == page_version_info->prev_version_.end()) {
    return std::nullopt; // 槽位不存在
  }

  // 返回版本链接
  return slot_it->second;
}

/** @brief 访问事务撤销日志缓冲区并获取撤销日志。如果事务不存在，返回 nullopt。 */
std::optional<UndoLog> TransactionManager::GetUndoLogOptional(UndoLink link) {
  std::shared_lock<std::shared_mutex> lock(txn_map_mutex_);

  // 查找事务
  auto txn_it = txn_map.find(link.prev_txn_);
  if (txn_it == txn_map.end()) {
    return std::nullopt;
  }

  Transaction *txn = txn_it->second;

  // 确保索引在范围内
  if (link.prev_log_idx_ >= txn->GetUndoLogNum()) {
    throw std::out_of_range("Undo log index out of range");
  }

  // 获取撤销日志
  auto result = txn->GetUndoLog(link.prev_log_idx_);
  return result;
}

/** @brief 访问事务撤销日志缓冲区并获取撤销日志。 */
UndoLog TransactionManager::GetUndoLog(UndoLink link) {
  std::shared_lock<std::shared_mutex> lock(txn_map_mutex_);

  // 查找事务
  auto txn_it = txn_map.find(link.prev_txn_);
  if (txn_it == txn_map.end()) {
    throw TransactionAbortException(INVALID_TXN_ID, AbortReason::DEADLOCK_PREVENTION);
  }

  Transaction *txn = txn_it->second;

  // 确保索引在范围内
  if (link.prev_log_idx_ >= txn->GetUndoLogNum()) {
    throw std::out_of_range("Undo log index out of range");
  }

  // 获取撤销日志
  return txn->GetUndoLog(link.prev_log_idx_);
}

/** @brief 垃圾回收清理无用的版本链 */
void TransactionManager::GarbageCollection() {
  timestamp_t watermark = GetWatermark();

  std::shared_lock<std::shared_mutex> read_lock(version_info_mutex_);
  for (auto &page_pair : version_info_) {
    auto page_version_info = page_pair.second;
    std::unique_lock<std::shared_mutex> page_lock(page_version_info->mutex_);

    auto it = page_version_info->prev_version_.begin();
    while (it != page_version_info->prev_version_.end()) {
      auto &version_link = it->second;
      bool can_remove = false;

      if (!version_link.in_progress_) {
        if (version_link.commit_ts_ != INVALID_TS) {
          if (version_link.commit_ts_ < watermark) {
            can_remove = true;
          }
        } else {
          try {
            auto log_opt = GetUndoLogOptional(version_link.prev_);
            if (log_opt.has_value()) {
              if (log_opt->ts_ < watermark) {
                can_remove = true;
              }
            } else {
              can_remove = true;
            }
          } catch (const std::exception &e) {
            can_remove = true;
          }
        }
      }

      if (can_remove) {
        it = page_version_info->prev_version_.erase(it);
      } else {
        ++it;
      }
    }

    if (page_version_info->prev_version_.empty()) {
      // 可以考虑移除整个页面版本信息，但需要小心并发访问
    }
  }
}

timestamp_t TransactionManager::GetNextTimestamp() {
  // 原子递增时间戳计数器，保证线程安全
  return next_timestamp_++;
}

/** @brief 检查记录对事务的可见性 */
bool TransactionManager::IsVisible(Rid rid, Transaction *txn) {
  if (concurrency_mode_ != ConcurrencyMode::MVCC) {
    return true; // 非MVCC模式，直接可见
  }

  // 获取版本链接
  auto version_link_opt = GetVersionLink(rid);
  if (!version_link_opt.has_value()) {
    return true; // 没有版本链，记录可见（正常记录）
  }

  auto version_link = *version_link_opt;

  // 如果版本正在进行中（未提交）
  if (version_link.in_progress_) {
    // 只有创建该版本的事务能看到未提交的修改
    if (version_link.prev_.prev_txn_ == txn->get_transaction_id()) {
      return true; // 当前事务能看到自己的修改
    }
    // 其他事务看不到未提交的修改，但需要重构历史版本，所以返回true让ReconstructTuple处理
    return true;
  }

  // 对于已提交的版本，记录总是可见的（可能是当前版本或历史版本）
  // 具体返回哪个版本由ReconstructTuple决定
  return true;
}

/** @brief 重构可见版本的记录 */
std::optional<RmRecord> TransactionManager::ReconstructTuple(Rid rid,
                                                             Transaction *txn) {
  if (concurrency_mode_ != ConcurrencyMode::MVCC) {
    return std::nullopt; // 非MVCC模式不需要重构
  }

  // 获取版本链接
  auto version_link_opt = GetVersionLink(rid);
  if (!version_link_opt.has_value()) {
    return std::nullopt; // 没有版本链，使用原始记录
  }

  auto version_link = *version_link_opt;

  // 如果版本正在进行中（未提交）
  if (version_link.in_progress_) {
    // 如果是当前事务的修改，使用当前版本
    if (version_link.prev_.prev_txn_ == txn->get_transaction_id()) {
      return std::nullopt; // 返回nullopt让调用者使用当前记录
    }

    // 如果是其他事务的未提交修改，需要重构历史版本
    try {
      UndoLog log = GetUndoLog(version_link.prev_);
      if (log.tuple_test_) {
        // 使用保存的原始记录
        auto reconstructed = std::make_unique<RmRecord>(log.tuple_test_->size);
        memcpy(reconstructed->data, log.tuple_test_->data, log.tuple_test_->size);
        return *reconstructed;
      }
    } catch (const std::exception &e) {
      // 无法获取历史版本，返回空
      return std::nullopt;
    }
  }

  // 对于已提交的版本，检查时间戳可见性
  timestamp_t commit_ts = version_link.commit_ts_;
  if (commit_ts == INVALID_TS) {
    try {
      UndoLog log = GetUndoLog(version_link.prev_);
      commit_ts = log.ts_;
    } catch (const std::exception &e) {
      return std::nullopt;
    }
  }

  // 如果是当前事务创建的记录，使用当前版本
  if (version_link.prev_.prev_txn_ == txn->get_transaction_id()) {
    return std::nullopt;
  }

  // 添加调试信息
  if (commit_ts > txn->get_start_ts()) {
    // 打印调试信息（在生产环境中应该移除）
    std::printf("ReconstructTuple: commit_ts=%lu > start_ts=%lu, need reconstruction\n", 
                commit_ts, txn->get_start_ts());
  }

  // 快照隔离：如果记录在事务开始后提交，需要重构历史版本
  if (commit_ts > txn->get_start_ts()) {
    std::printf("ReconstructTuple: 尝试重建历史版本\n");
    
    // 首先尝试使用version_link中的历史数据（推荐方式）
    if (version_link.historical_data_) {
      std::printf("ReconstructTuple: 从version_link.historical_data_重建历史版本\n");
      auto reconstructed = std::make_unique<RmRecord>(version_link.historical_data_->size);
      memcpy(reconstructed->data, version_link.historical_data_->data, version_link.historical_data_->size);
      return *reconstructed;
    }
    
    // 如果historical_data_不可用，尝试从UndoLog获取
    try {
      UndoLog log = GetUndoLog(version_link.prev_);
      std::printf("ReconstructTuple: log.tuple_test_ = %s\n", log.tuple_test_ ? "有数据" : "NULL");
      if (log.tuple_test_) {
        std::printf("ReconstructTuple: 从UndoLog重建历史版本\n");
        auto reconstructed = std::make_unique<RmRecord>(log.tuple_test_->size);
        memcpy(reconstructed->data, log.tuple_test_->data, log.tuple_test_->size);
        return *reconstructed;
      }
    } catch (const std::exception &e) {
      std::printf("ReconstructTuple: 无法访问UndoLog - %s，这是正常的（事务已结束）\n", e.what());
    }
    
    std::printf("ReconstructTuple: 警告 - 无法找到历史数据！\n");
  }

  // 记录在事务开始前提交，使用当前版本
  return std::nullopt;
}

/** @brief 获取水位线时间戳，用于垃圾回收 */
timestamp_t TransactionManager::GetWatermark() {
  return running_txns_.GetWatermark();
}

/** @brief 更新事务提交时的所有版本链状态 */
void TransactionManager::UpdateTransactionVersionLinks(txn_id_t txn_id, timestamp_t commit_ts) {
  std::shared_lock<std::shared_mutex> read_lock(version_info_mutex_);
  
  // 遍历所有页面的版本信息
  for (auto& page_pair : version_info_) {
    auto page_version_info = page_pair.second;
    std::unique_lock<std::shared_mutex> page_lock(page_version_info->mutex_);
    
    // 遍历该页面的所有槽位
    for (auto& slot_pair : page_version_info->prev_version_) {
      auto& version_link = slot_pair.second;
      
      // 如果这个版本链是由指定事务创建的，并且仍在进行中
      if (version_link.in_progress_ && version_link.prev_.prev_txn_ == txn_id) {
        // 设置为不再进行中，并缓存提交时间戳
        version_link.in_progress_ = false;
        version_link.commit_ts_ = commit_ts;
      }
    }
  }
}




