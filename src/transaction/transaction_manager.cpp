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
    txn_id_t txn_id = next_txn_id_++;
    std::cout << "[DEBUG] BEGIN: 创建新事务，txn_id=" << txn_id << std::endl;
    
    // 永远创建新事务
    Transaction* new_txn = new Transaction(txn_id);
    new_txn->set_state(TransactionState::GROWING);
    
    // 显式设置事务模式
    bool is_explicit = (txn != nullptr && txn->get_txn_mode());
    new_txn->set_txn_mode(is_explicit);
    
    // 添加到事务表
    {
        std::unique_lock<std::mutex> lock(latch_);
        txn_map[txn_id] = new_txn;
    }
    
    // 写日志
    if (log_manager != nullptr) {
        try {
            BeginLogRecord log_record(txn_id);
            log_record.prev_lsn_ = INVALID_LSN;  
            lsn_t lsn = log_manager->add_log_to_buffer(&log_record); 
            new_txn->set_prev_lsn(lsn);
        } catch (const std::exception& e) {
            std::cerr << "创建BEGIN日志失败: " << e.what() << std::endl;
        }
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
    // // 更新事务状态为已提交
    // txn->set_state(TransactionState::COMMITTED);
    
    // // 从事务表中移除
    // if (log_success) {
    //     std::unique_lock<std::mutex> lock(latch_);
    //     txn_map.erase(txn->get_transaction_id());
    // } else {
    //     // 如果日志失败，保留事务以便后续尝试
    //     std::cerr << "Warning: Transaction remains in txn_map due to log failure" << std::endl;
    // }
    // if (txn == nullptr) {
    //     std::cerr << "警告: 尝试提交空事务" << std::endl;
    //     return;
    // }
    
    // // 检查事务状态
    // if (txn->get_state() != TransactionState::GROWING) {
    //     std::cerr << "警告: 尝试提交状态错误的事务 ID=" << txn->get_transaction_id() << std::endl;
    //     return;
    // }

    // bool log_success = true;
    // // 创建commit日志
    // if (log_manager != nullptr) {
    //     try {
    //         auto commit_log = new CommitLogRecord(txn->get_transaction_id());
    //         commit_log->prev_lsn_ = txn->get_prev_lsn();
    //         lsn_t lsn = log_manager->add_log_to_buffer(commit_log);
    //         txn->set_prev_lsn(lsn);
    //         delete commit_log;
    //     } catch (std::exception& e) {
    //         std::cerr << "创建提交日志失败: " << e.what() << std::endl;
    //         log_success = false;
    //     }
    // }

    // try {
    //     // 设置事务状态为已提交
    //     txn->set_state(TransactionState::COMMITTED);
        
    //     // 释放所有锁
    //     if (lock_manager_ != nullptr) {
    //         lock_manager_->unlock_all(txn);
    //     }
        
    //     // 刷新日志
    //     if (log_manager != nullptr && log_success) {
    //         log_manager->flush_log_to_disk();
    //     }
        
    //     // 从事务表中移除事务
    //     if (log_success) {
    //         std::unique_lock<std::mutex> lock(latch_);
    //         txn_map.erase(txn->get_transaction_id());
    //     } else {
    //         std::cerr << "警告: 由于日志错误，事务仍保留在txn_map中" << std::endl;
    //     }
    // } catch (std::exception& e) {
    //     std::cerr << "提交事务出错: " << e.what() << std::endl;
    // }
    std::cout << "[DEBUG] COMMIT: 提交事务，txn_id=" << (txn ? txn->get_transaction_id() : -1) << std::endl;

    if (txn == nullptr) {
        std::cerr << "错误: 尝试提交空事务" << std::endl;
        return;
    }
    
    // 检查事务状态
    if (txn->get_state() != TransactionState::GROWING) {
        std::cerr << "错误: 尝试提交非GROWING状态的事务 ID=" << txn->get_transaction_id() << std::endl;
        return;
    }

    bool log_success = true;
    
    // 1. 记录提交日志
    if (log_manager != nullptr) {
        try {
            CommitLogRecord commit_log(txn->get_transaction_id());
            commit_log.prev_lsn_ = txn->get_prev_lsn();  
            lsn_t lsn = log_manager->add_log_to_buffer(&commit_log); 
            txn->set_prev_lsn(lsn);
        } catch (std::exception& e) {
            std::cerr << "创建提交日志失败: " << e.what() << std::endl;
            log_success = false;
        }
    }

    // 2. 更新事务状态并清理资源
    try {
        // 设置事务状态为已提交
        txn->set_state(TransactionState::COMMITTED);
        
        // 清理写集合中的对象以防内存泄漏
        auto write_set = txn->get_write_set();
        while (!write_set->empty()) {
            WriteRecord* record = write_set->front();
            write_set->pop_front();
            delete record;
        }
        
        // 释放所有锁
        if (lock_manager_ != nullptr) {
            lock_manager_->unlock_all(txn);
        }
        
        // 刷新日志到磁盘
        if (log_manager != nullptr && log_success) {
            log_manager->flush_log_to_disk();
        }
        
        // 从事务表中移除事务
        if (log_success) {
            std::unique_lock<std::mutex> lock(latch_);
            txn_map.erase(txn->get_transaction_id());
        } else {
            std::cerr << "警告: 由于日志错误，事务仍保留在txn_map中" << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "提交事务时出现异常: " << e.what() << std::endl;
        // 在异常情况下，仍然尝试从事务表中移除
        try {
            std::unique_lock<std::mutex> lock(latch_);
            txn_map.erase(txn->get_transaction_id());
        } catch (...) {
            // 忽略任何异常，确保函数返回
        }
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

    // if (txn == nullptr) {
    //     std::cerr << "警告: 尝试回滚空事务" << std::endl;
    //     return;
    // }
    
    // bool log_success = true;
    // // 记录回滚日志
    // if (log_manager != nullptr) {
    //     try {
    //         auto abort_log = new AbortLogRecord(txn->get_transaction_id());
    //         abort_log->prev_lsn_ = txn->get_prev_lsn();
    //         lsn_t lsn = log_manager->add_log_to_buffer(abort_log);
    //         txn->set_prev_lsn(lsn);
    //         delete abort_log;
    //     } catch (std::exception& e) {
    //         std::cerr << "创建回滚日志失败: " << e.what() << std::endl;
    //         log_success = false;
    //     }
    // }

    // // 回滚所有写操作 - 修复迭代器问题
    // std::deque<WriteRecord*>& write_set = *(txn->get_write_set());
    // for (auto it = write_set.rbegin(); it != write_set.rend(); ++it) {
    //     auto write_record = *it;
    //     std::string table_name = write_record->GetTableName();
        
    //     try {
    //         // 获取表文件句柄
    //         if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
    //             std::cerr << "表不存在: " << table_name << std::endl;
    //             continue;
    //         }
    //         auto fh = sm_manager_->fhs_.at(table_name).get();
            
    //         // 获取表结构和索引
    //         auto& table_meta = sm_manager_->db_.get_table(table_name);
            
    //         if (write_record->GetWriteType() == WType::INSERT_TUPLE) {
    //             // 删除索引
    //             for (auto& index : table_meta.indexes) {
    //                 // 使用getter方法获取ix_manager
    //                 std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
    //                 if (sm_manager_->ihs_.find(index_name) == sm_manager_->ihs_.end()) {
    //                     std::cerr << "索引不存在: " << index_name << std::endl;
    //                     continue;
    //                 }
    //                 auto ih = sm_manager_->ihs_.at(index_name).get();
    //                 // 删除索引条目 - 直接使用txn，不使用Context
    //                 ih->delete_entry(write_record->GetRecord().data, txn);
    //             }
    //             // 删除记录 - 需要先创建Context
    //             Context delete_context(lock_manager_, log_manager, txn);
    //             fh->delete_record(write_record->GetRid(), &delete_context);
    //         } 
    //         else if (write_record->GetWriteType() == WType::DELETE_TUPLE) {
    //             // 恢复记录 - 没有需要传入Transaction的版本
    //             fh->insert_record(write_record->GetRid(), write_record->GetRecord().data);
    //             // 恢复索引
    //             for (auto& index : table_meta.indexes) {
    //                 std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
    //                 if (sm_manager_->ihs_.find(index_name) == sm_manager_->ihs_.end()) {
    //                     std::cerr << "索引不存在: " << index_name << std::endl;
    //                     continue;
    //                 }
    //                 auto ih = sm_manager_->ihs_.at(index_name).get();
    //                 // 直接使用txn，不使用Context
    //                 ih->insert_entry(write_record->GetRecord().data, write_record->GetRid(), txn);
    //             }
    //         }
    //         else if (write_record->GetWriteType() == WType::UPDATE_TUPLE) {
    //             // 恢复记录 - 使用Context
    //             Context update_context(lock_manager_, log_manager, txn);
    //             fh->update_record(write_record->GetRid(), write_record->GetRecord().data, &update_context);
                
    //             // 因为没有新旧记录区分，只是简单地删除和重建索引
    //             for (auto& index : table_meta.indexes) {
    //                 std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
    //                 if (sm_manager_->ihs_.find(index_name) == sm_manager_->ihs_.end()) {
    //                     std::cerr << "索引不存在: " << index_name << std::endl;
    //                     continue;
    //                 }
    //                 auto ih = sm_manager_->ihs_.at(index_name).get();
                    
    //                 // 直接使用txn，不使用Context
    //                 ih->delete_entry(write_record->GetRecord().data, txn);
    //                 ih->insert_entry(write_record->GetRecord().data, write_record->GetRid(), txn);
    //             }
    //         }
    //     } catch (std::exception& e) {
    //         std::cerr << "回滚操作出错: " << e.what() << " 表名: " << table_name << std::endl;
    //     }
    // }

    // // 设置事务状态
    // txn->set_state(TransactionState::ABORTED);
    
    // // 释放所有锁
    // if (lock_manager_ != nullptr) {
    //     lock_manager_->unlock_all(txn);
    // }
    
    // // 刷新日志
    // if (log_manager != nullptr && log_success) {
    //     log_manager->flush_log_to_disk();
    // }
    
    // // 从事务表中移除事务
    // if (log_success) {
    //     std::unique_lock<std::mutex> lock(latch_);
    //     txn_map.erase(txn->get_transaction_id());
    // } else {
    //     std::cerr << "警告: 由于日志错误，事务仍保留在txn_map中" << std::endl;
    // }
    std::cout << "[DEBUG] ABORT: 回滚事务，txn_id=" << (txn ? txn->get_transaction_id() : -1) << std::endl;
    
    if (txn == nullptr) {
        std::cerr << "[严重错误] 尝试回滚空事务指针" << std::endl;
        return;
    }
    
    if (txn->get_write_set() == nullptr) {
        std::cerr << "[严重错误] 事务的写集合为空指针" << std::endl;
        txn->set_state(TransactionState::ABORTED);
        return;
    }
    
    bool log_success = true;
    
    // 1. 记录回滚日志
    if (log_manager != nullptr) {
        try {
            AbortLogRecord abort_log(txn->get_transaction_id());
            abort_log.prev_lsn_ = txn->get_prev_lsn(); 
            lsn_t lsn = log_manager->add_log_to_buffer(&abort_log); 
            txn->set_prev_lsn(lsn);
        } catch (std::exception& e) {
            std::cerr << "创建回滚日志失败: " << e.what() << std::endl;
            log_success = false;
        }
    }

    try {
        // 2. 回滚所有写操作 - 从后向前处理确保正确的顺序
        std::deque<WriteRecord*>& write_set = *(txn->get_write_set());
        std::vector<WriteRecord*> records_to_delete;
        std::cout << "[DEBUG] 开始回滚写记录，共" << write_set.size() << "条" << std::endl;        // 复制所有记录，避免迭代器失效问题
        for (auto it = write_set.rbegin(); it != write_set.rend(); ++it) {
            records_to_delete.push_back(*it);
        }
        
        // 清空原始写集合
        write_set.clear();
        
        // 处理每条记录
        for (auto write_record : records_to_delete) {
            std::string table_name = write_record->GetTableName();
            
            try {
                // 获取表文件句柄
                if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
                    std::cerr << "表不存在: " << table_name << std::endl;
                    continue;
                }
                auto fh = sm_manager_->fhs_.at(table_name).get();
                
                // 获取表结构
                auto& table_meta = sm_manager_->db_.get_table(table_name);
                
                switch (write_record->GetWriteType()) {
                    case WType::INSERT_TUPLE: {
                        // 处理索引
                        for (auto& index : table_meta.indexes) {
                            try {
                                // 安全获取索引句柄
                                std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
                                if (sm_manager_->ihs_.find(index_name) == sm_manager_->ihs_.end() || !sm_manager_->ihs_.at(index_name)) {
                                    std::cerr << "索引不存在或无效: " << index_name << std::endl;
                                    continue;
                                }
                                auto ih = sm_manager_->ihs_.at(index_name).get();
                                
                                // 检查记录数据是否为空
                                if (!write_record->GetRecord().data) {
                                    std::cerr << "记录数据为空: 表=" << table_name << ", RID=(" 
                                              << write_record->GetRid().page_no << "," << write_record->GetRid().slot_no << ")" << std::endl;
                                    continue;
                                }
                                
                                // 使用智能指针管理索引键内存
                                std::unique_ptr<char[]> key_ptr(new char[index.col_tot_len]);
                                char* key = key_ptr.get();
                                memset(key, 0, index.col_tot_len); // 初始化内存
                                
                                // 安全构建索引键
                                bool key_valid = true;
                                int offset = 0;
                                for (size_t j = 0; j < index.col_num; j++) {
                                    // 检查索引列是否在记录范围内
                                    if (index.cols[j].offset >= write_record->GetRecord().size || 
                                        index.cols[j].offset + index.cols[j].len > write_record->GetRecord().size) {
                                        std::cerr << "索引列超出记录范围: 表=" << table_name 
                                                  << ", 列偏移=" << index.cols[j].offset 
                                                  << ", 列长度=" << index.cols[j].len 
                                                  << ", 记录大小=" << write_record->GetRecord().size << std::endl;
                                        key_valid = false;
                                        break;
                                    }
                                    
                                    // 复制索引键数据
                                    memcpy(key + offset, 
                                           write_record->GetRecord().data + index.cols[j].offset, 
                                           index.cols[j].len);
                                    offset += index.cols[j].len;
                                }
                                
                                // 只有在键有效时才删除索引
                                if (key_valid) {
                                    ih->delete_entry(key, txn);
                                    std::cout << "[DEBUG] 删除索引项: 表=" << table_name 
                                              << ", 索引=" << index_name << std::endl;
                                }
                            } catch (std::exception& e) {
                                std::cerr << "删除索引项异常: " << e.what() << std::endl;
                                // 继续处理其他索引
                            } catch (...) {
                                std::cerr << "删除索引项未知异常" << std::endl;
                            }
                        }
                        
                        // 删除记录
                        Context delete_context(lock_manager_, log_manager, txn);
                        fh->delete_record(write_record->GetRid(), &delete_context);
                        break;
                    }
                    
                    case WType::DELETE_TUPLE: {
                        // 恢复记录
                        try {
                            // 恢复记录，确保记录数据有效
                            if (!write_record->GetRecord().data) {
                                std::cerr << "记录数据为空，无法恢复记录" << std::endl;
                                break;
                            }
                            
                            fh->insert_record(write_record->GetRid(), write_record->GetRecord().data);
                            std::cout << "[DEBUG] 恢复删除的记录: RID=(" 
                                      << write_record->GetRid().page_no << "," 
                                      << write_record->GetRid().slot_no << ")" << std::endl;
                            
                            // 恢复索引
                            for (auto& index : table_meta.indexes) {
                                try {
                                    std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
                                    if (sm_manager_->ihs_.find(index_name) == sm_manager_->ihs_.end() || !sm_manager_->ihs_.at(index_name)) {
                                        std::cerr << "索引不存在或无效: " << index_name << std::endl;
                                        continue;
                                    }
                                    auto ih = sm_manager_->ihs_.at(index_name).get();
                                    
                                    // 使用智能指针管理索引键内存
                                    std::unique_ptr<char[]> key_ptr(new char[index.col_tot_len]);
                                    char* key = key_ptr.get();
                                    memset(key, 0, index.col_tot_len); // 初始化内存
                                    
                                    // 安全构建索引键
                                    bool key_valid = true;
                                    int offset = 0;
                                    for (size_t j = 0; j < index.col_num; j++) {
                                        // 检查索引列是否在记录范围内
                                        if (index.cols[j].offset >= write_record->GetRecord().size || 
                                            index.cols[j].offset + index.cols[j].len > write_record->GetRecord().size) {
                                            std::cerr << "索引列超出记录范围: 表=" << table_name 
                                                      << ", 列偏移=" << index.cols[j].offset 
                                                      << ", 列长度=" << index.cols[j].len 
                                                      << ", 记录大小=" << write_record->GetRecord().size << std::endl;
                                            key_valid = false;
                                            break;
                                        }
                                        
                                        // 复制索引键数据
                                        memcpy(key + offset, 
                                               write_record->GetRecord().data + index.cols[j].offset, 
                                               index.cols[j].len);
                                        offset += index.cols[j].len;
                                    }
                                    
                                    // 只有在键有效时才插入索引
                                    if (key_valid) {
                                        ih->insert_entry(key, write_record->GetRid(), txn);
                                        std::cout << "[DEBUG] 恢复索引项: 表=" << table_name 
                                                  << ", 索引=" << index_name << std::endl;
                                    }
                                } catch (std::exception& e) {
                                    std::cerr << "恢复索引项异常: " << e.what() << std::endl;
                                    // 继续处理其他索引
                                } catch (...) {
                                    std::cerr << "恢复索引项未知异常" << std::endl;
                                }
                            }
                        } catch (std::exception& e) {
                            std::cerr << "恢复记录异常: " << e.what() << std::endl;
                        } catch (...) {
                            std::cerr << "恢复记录未知异常" << std::endl;
                        }
                        break;
                    }
                    
                    case WType::UPDATE_TUPLE: {
                        // 对于UPDATE，原始记录包含更新前的数据
                        try {
                            // 1. 创建操作上下文
                            Context update_context(lock_manager_, log_manager, txn);
                            
                            // 2. 安全获取当前记录
                            std::unique_ptr<RmRecord> current_record_ptr;
                            try {
                                current_record_ptr = fh->get_record(write_record->GetRid(), &update_context);
                                if (!current_record_ptr) {
                                    std::cerr << "无法获取当前记录: " << write_record->GetRid().page_no 
                                              << "," << write_record->GetRid().slot_no << std::endl;
                                    continue;
                                }
                            } catch (std::exception& e) {
                                std::cerr << "获取当前记录失败: " << e.what() << std::endl;
                                continue;
                            }
                            
                            // 3. 处理每个索引
                            for (auto& index : table_meta.indexes) {
                                try {
                                    std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
                                    if (sm_manager_->ihs_.find(index_name) == sm_manager_->ihs_.end() ||
                                        !sm_manager_->ihs_.at(index_name)) {
                                        continue;
                                    }
                                    auto ih = sm_manager_->ihs_.at(index_name).get();
                                    
                                    // 4. 使用智能指针管理索引键内存
                                    std::unique_ptr<char[]> current_key_ptr(new char[index.col_tot_len]);
                                    std::unique_ptr<char[]> original_key_ptr(new char[index.col_tot_len]);
                                    char* current_key = current_key_ptr.get();
                                    char* original_key = original_key_ptr.get();
                                    
                                    // 5. 初始化内存
                                    memset(current_key, 0, index.col_tot_len);
                                    memset(original_key, 0, index.col_tot_len);
                                    
                                    // 6. 安全构建索引键
                                    bool keys_valid = true;
                                    int offset = 0;
                                    
                                    // 构建当前记录的键
                                    for (size_t j = 0; j < index.col_num && keys_valid; j++) {
                                        if (index.cols[j].offset >= current_record_ptr->size || 
                                            index.cols[j].offset + index.cols[j].len > current_record_ptr->size) {
                                            keys_valid = false;
                                            break;
                                        }
                                        
                                        memcpy(current_key + offset, 
                                               current_record_ptr->data + index.cols[j].offset, 
                                               index.cols[j].len);
                                        offset += index.cols[j].len;
                                    }
                                    
                                    // 构建原始记录的键
                                    offset = 0;
                                    for (size_t j = 0; j < index.col_num && keys_valid; j++) {
                                        if (index.cols[j].offset >= write_record->GetRecord().size || 
                                            index.cols[j].offset + index.cols[j].len > write_record->GetRecord().size) {
                                            keys_valid = false;
                                            break;
                                        }
                                        
                                        memcpy(original_key + offset, 
                                               write_record->GetRecord().data + index.cols[j].offset, 
                                               index.cols[j].len);
                                        offset += index.cols[j].len;
                                    }
                                    
                                    // 7. 只有在键有效时才执行索引操作
                                    if (keys_valid) {
                                        ih->delete_entry(current_key, txn);
                                        ih->insert_entry(original_key, write_record->GetRid(), txn);
                                        std::cout << "[DEBUG] 更新索引项: 表=" << table_name 
                                                  << ", 索引=" << index_name << std::endl;
                                    }
                                } catch (std::exception& e) {
                                    std::cerr << "更新索引异常: " << e.what() << std::endl;
                                }
                            }
                            
                            // 8. 更新记录
                            fh->update_record(write_record->GetRid(), write_record->GetRecord().data, &update_context);
                            std::cout << "[DEBUG] 成功回滚更新记录: " << write_record->GetRid().page_no 
                                      << "," << write_record->GetRid().slot_no << std::endl;
                                      
                        } catch (std::exception& e) {
                            std::cerr << "回滚更新记录异常: " << e.what() << std::endl;
                        }
                        break;
                    }
                }
            } catch (std::exception& e) {
                std::cerr << "回滚操作失败: " << e.what() << " 表: " << table_name << std::endl;
            }
            
            // 释放写记录内存
            delete write_record;
        }
        
        // 3. 设置事务状态
        txn->set_state(TransactionState::ABORTED);
        
        // 4. 释放所有锁
        if (lock_manager_ != nullptr) {
            lock_manager_->unlock_all(txn);
        }
        
        // 5. 刷新日志到磁盘
        if (log_manager != nullptr && log_success) {
            log_manager->flush_log_to_disk();
        }
        
        // 6. 从事务表中移除事务
        if (log_success) {
            std::unique_lock<std::mutex> lock(latch_);
            txn_map.erase(txn->get_transaction_id());
        } else {
            std::cerr << "警告: 由于日志错误，事务仍保留在txn_map中" << std::endl;
        }
    } catch (std::exception& e) {
        std::cerr << "回滚事务时出现异常: " << e.what() << std::endl;
        
        // 在异常情况下，仍然尝试从事务表中移除
        try {
            std::unique_lock<std::mutex> lock(latch_);
            txn_map.erase(txn->get_transaction_id());
        } catch (...) {
            // 忽略任何异常，确保函数返回
        }
    }
}


