/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "log_recovery.h"
#include "record/rm_manager.h"
#include "recovery/log_manager.h"
#include "system/sm_manager.h"
/**#
 * @description: analyze阶段，需要获得脏页表（DPT）和未完成的事务列表（ATT）
 */
 void RecoveryManager::analyze() {
    std::cout << "开始执行分析阶段..." << std::endl;
    
    // 获取所有表名并打开表文件
    std::vector<std::string> tables;
    sm_manager_->get_all_tables(tables);
    
    for (const auto& table_name : tables) {
        if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
            try {
                sm_manager_->fhs_[table_name] = sm_manager_->open_table_file(table_name);
                std::cout << "恢复前打开表: " << table_name << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "打开表失败: " << table_name << " - " << e.what() << std::endl;
            }
        }
    }
    
    // ✅ 修复：使用正确的方法获取检查点
    lsn_t checkpoint_lsn = INVALID_LSN;
    try {
        // 尝试获取最后一个检查点LSN
        checkpoint_lsn = log_manager_->get_last_checkpoint_lsn();
        if (checkpoint_lsn == INVALID_LSN) {
            std::cout << "未找到检查点，将从头开始恢复" << std::endl;
        } else {
            std::cout << "从检查点LSN " << checkpoint_lsn << " 开始恢复" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "获取检查点失败，将从头开始恢复: " << e.what() << std::endl;
        checkpoint_lsn = INVALID_LSN;
    }
    
    // ✅ 扫描日志（如果没有检查点就从头开始）
    // ✅ 扫描日志（如果没有检查点就从头开始）
    std::vector<LogRecord*> log_records;
    if (checkpoint_lsn == INVALID_LSN) {
        log_records = log_manager_->scan_log_from_lsn();  // 从头开始
    } else {
        log_records = log_manager_->scan_log_from_lsn(checkpoint_lsn);  // 从检查点开始
    }
    
    std::cout << "从日志中读取了 " << log_records.size() << " 条记录" << std::endl;
    
    // ✅ 修复：完整扫描确定最终事务状态
    std::unordered_set<txn_id_t> committed_txns;
    std::unordered_set<txn_id_t> aborted_txns;
    std::unordered_map<txn_id_t, lsn_t> final_active_txns;  // 最终活跃的事务
    
    // **一遍扫描确定所有事务的最终状态**
    for (auto* record : log_records) {
        std::cout << "日志记录: 类型=" << static_cast<int>(record->log_type_) 
                  << ", 事务ID=" << record->log_tid_ 
                  << ", LSN=" << record->lsn_ << std::endl;
        
        txn_id_t txn_id = record->log_tid_;
        
        switch (record->log_type_) {
            case LogType::begin:
                final_active_txns[txn_id] = record->lsn_;
                std::cout << "发现开始事务: " << txn_id << std::endl;
                break;
                
            case LogType::commit:
                final_active_txns.erase(txn_id);  // 从活跃表移除
                committed_txns.insert(txn_id);    // 加入提交表
                aborted_txns.erase(txn_id);       // 防止状态冲突
                std::cout << "发现已提交事务: " << txn_id << std::endl;
                break;
                
            case LogType::ABORT:
                final_active_txns.erase(txn_id);  // 从活跃表移除
                aborted_txns.insert(txn_id);      // 加入中止表
                committed_txns.erase(txn_id);     // 防止状态冲突
                std::cout << "发现已中止事务: " << txn_id << std::endl;
                break;
                
            case LogType::INSERT:
            case LogType::UPDATE:
            case LogType::DELETE:
                // 更新活跃事务的最新LSN（如果事务仍然活跃）
                if (final_active_txns.find(txn_id) != final_active_txns.end()) {
                    final_active_txns[txn_id] = record->lsn_;
                }
                std::cout << "处理数据修改日志: 类型=" << static_cast<int>(record->log_type_) 
                          << ", 事务ID=" << txn_id 
                          << ", LSN=" << record->lsn_ << std::endl;
                break;
                
            default:
                break;
        }
    }
    
    // ✅ 现在final_active_txns包含真正活跃的事务
    active_txn_table_ = final_active_txns;
    
    // ✅ 第二遍扫描：只为已提交或最终活跃事务构建脏页表
    for (auto* record : log_records) {
        if (record->log_type_ == LogType::INSERT || 
            record->log_type_ == LogType::UPDATE || 
            record->log_type_ == LogType::DELETE) {
            
            txn_id_t txn_id = record->log_tid_;
            
            // ✅ 关键修复：使用最终状态检查
            bool should_add_to_dirty_pages = false;
            
            if (committed_txns.find(txn_id) != committed_txns.end()) {
                // 已提交事务，需要重做
                should_add_to_dirty_pages = true;
                std::cout << "已提交事务的操作: 事务" << txn_id << ", LSN=" << record->lsn_ << std::endl;
            } else if (final_active_txns.find(txn_id) != final_active_txns.end()) {
                // 最终仍然活跃的事务，也需要重做（稍后会被UNDO）
                should_add_to_dirty_pages = true;
                std::cout << "活跃事务的操作: 事务" << txn_id << ", LSN=" << record->lsn_ << std::endl;
            } else if (aborted_txns.find(txn_id) != aborted_txns.end()) {
                // ✅ 已中止事务，不加入脏页表
                should_add_to_dirty_pages = false;
                std::cout << "跳过已中止事务的操作: 事务" << txn_id << ", LSN=" << record->lsn_ << std::endl;
                continue;  // 直接跳过
            }
            
            if (!should_add_to_dirty_pages) {
                continue;
            }
            
            // 构建脏页表的逻辑保持不变...
            Rid rid;
            bool has_valid_rid = false;
            std::string table_name;
            
            if (record->log_type_ == LogType::INSERT) {
                InsertLogRecord* insert_log = dynamic_cast<InsertLogRecord*>(record);
                if (insert_log) {
                    rid = insert_log->rid_;
                    table_name = insert_log->table_name_;
                    has_valid_rid = true;
                    std::cout << "处理INSERT: 表=" << table_name 
                              << ", RID=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                }
            } else if (record->log_type_ == LogType::UPDATE) {
                UpdateLogRecord* update_log = dynamic_cast<UpdateLogRecord*>(record);
                if (update_log) {
                    rid = update_log->rid_;
                    table_name = update_log->table_name_;
                    has_valid_rid = true;
                    std::cout << "处理UPDATE: 表=" << table_name 
                              << ", RID=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                }
            } else if (record->log_type_ == LogType::DELETE) {
                DeleteLogRecord* delete_log = dynamic_cast<DeleteLogRecord*>(record);
                if (delete_log) {
                    rid = delete_log->rid_;
                    table_name = delete_log->table_name_;
                    has_valid_rid = true;
                    std::cout << "处理DELETE: 表=" << table_name 
                              << ", RID=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                }
            }
            
            if (has_valid_rid && rid.page_no != INVALID_PAGE_ID) {
                if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
                    std::cerr << "警告: 表 " << table_name << " 不存在或未打开" << std::endl;
                    continue;
                }
                
                PageId page_id;
                page_id.fd = sm_manager_->fhs_[table_name]->GetFd();
                page_id.page_no = rid.page_no;
                
                if (dirty_page_table_.find(page_id) == dirty_page_table_.end() ||
                    dirty_page_table_[page_id] > record->lsn_) {
                    dirty_page_table_[page_id] = record->lsn_;
                    std::cout << "添加脏页: 表=" << table_name 
                              << ", 页号=" << rid.page_no 
                              << ", LSN=" << record->lsn_ << std::endl;
                }
            }
        }
    }
    
    std::cout << "活跃事务表大小: " << active_txn_table_.size() << std::endl;
    std::cout << "脏页表大小: " << dirty_page_table_.size() << std::endl;
    std::cout << "已提交事务数: " << committed_txns.size() << std::endl;
    std::cout << "已中止事务数: " << aborted_txns.size() << std::endl;
    
    // 清理日志记录
    for (auto* record : log_records) {
        delete record;
    }
    
    // 执行redo和undo
    redo();
    undo();
    
    // 清空表和活跃事务表
    active_txn_table_.clear();
    dirty_page_table_.clear();
    
    std::cout << "分析阶段完成" << std::endl;
    lsn_t max_lsn = 0;
    txn_id_t max_txn_id = 0;
    std::vector<LogRecord*> all_log_records= log_manager_->scan_log_from_lsn(0);
    for (auto* record : all_log_records) {
        if (record->lsn_ > max_lsn) max_lsn = record->lsn_;
        if (record->log_tid_ > max_txn_id) max_txn_id = record->log_tid_;
    }
    // 恢复流程最后，设置新的起始值
    log_manager_->set_global_lsn(max_lsn + 1);
    txn_mgr_->set_next_txn_id(max_txn_id + 1);

    std::cout << "恢复后设置global_lsn=" << (max_lsn + 1)
              << "，next_txn_id=" << (max_txn_id + 1) << std::endl;
}

/**
 * @description: 从最后一个检查点恢复
 */
 void RecoveryManager::recover_from_checkpoint() {
    std::cout << "从检查点开始恢复..." << std::endl;
    
    // 打开所有表
    std::vector<std::string> tables;
    sm_manager_->get_all_tables(tables);
    
    for (const auto& table_name : tables) {
        if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
            try {
                sm_manager_->fhs_[table_name] = sm_manager_->open_table_file(table_name);
                std::cout << "恢复前打开表: " << table_name << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "打开表失败: " << table_name << " - " << e.what() << std::endl;
            }
        }
    }
    
    
    lsn_t checkpoint_lsn = log_manager_->get_last_checkpoint_lsn();
    if (checkpoint_lsn == INVALID_LSN) {
        throw std::runtime_error("无效的检查点LSN");
    }
    
    // 🔄 第一步：读取检查点记录，获取活跃事务 → undo_list（严格按照题目要求）
    std::unordered_set<txn_id_t> undo_list;
    std::unordered_set<txn_id_t> redo_list; // 初始为空
    
    try {
        LogRecord* checkpoint_record = log_manager_->read_log_record(checkpoint_lsn);
        if (!checkpoint_record || checkpoint_record->log_type_ != LogType::CHECKPOINT) {
            throw std::runtime_error("LSN对应的记录不是检查点记录");
        }
        
        CheckpointLogRecord* checkpoint = dynamic_cast<CheckpointLogRecord*>(checkpoint_record);
        std::vector<txn_id_t> active_txns = checkpoint->get_active_txns();
        
        // 将检查点时的活跃事务加入undo_list（题目要求的步骤1）
        for (auto txn_id : active_txns) {
            undo_list.insert(txn_id);
            active_txn_table_[txn_id] = INVALID_LSN;
            std::cout << "从检查点添加到undo_list: 事务" << txn_id << std::endl;
        }
        
        delete checkpoint_record;
    } catch (const std::exception& e) {
        std::cerr << "读取检查点记录失败: " << e.what() << std::endl;
        throw;
    }
    
    // 🔄 第二步：从检查点开始扫描日志（题目要求的步骤2）
    
    std::vector<LogRecord*> log_records = log_manager_->scan_log_from_lsn(checkpoint_lsn + 1);
    
    for (auto* record : log_records) {
        txn_id_t txn_id = record->log_tid_;
        
        switch (record->log_type_) {
            case LogType::begin:
                // 新开始的事务加入undo_list
                undo_list.insert(txn_id);
                active_txn_table_[txn_id] = record->lsn_;
                std::cout << "新事务加入undo_list: " << txn_id << std::endl;
                break;
                
            case LogType::commit:
                // 事务提交：从undo_list移除，加入redo_list
                if (undo_list.find(txn_id) != undo_list.end()) {
                    undo_list.erase(txn_id);
                    redo_list.insert(txn_id);
                    std::cout << "事务" << txn_id << "提交: 移到redo_list" << std::endl;
                }
                active_txn_table_.erase(txn_id);
                break;
                
            case LogType::ABORT:
                undo_list.erase(txn_id);
                active_txn_table_.erase(txn_id);
                break;
                
            case LogType::UPDATE:
            case LogType::INSERT:
            case LogType::DELETE:
                // 更新活跃事务表
                if (active_txn_table_.find(txn_id) != active_txn_table_.end()) {
                    active_txn_table_[txn_id] = record->lsn_;
                }
                
                // 只为redo_list中的事务构建脏页表
                if (redo_list.find(txn_id) != redo_list.end()) {
                    Rid rid;
                    std::string table_name;
                    bool has_valid_rid = false;
                    
                    // 提取记录信息
                    if (record->log_type_ == LogType::INSERT) {
                        InsertLogRecord* insert_log = dynamic_cast<InsertLogRecord*>(record);
                        if (insert_log) {
                            rid = insert_log->rid_;
                            table_name = insert_log->table_name_;
                            has_valid_rid = true;
                        }
                    } else if (record->log_type_ == LogType::UPDATE) {
                        UpdateLogRecord* update_log = dynamic_cast<UpdateLogRecord*>(record);
                        if (update_log) {
                            rid = update_log->rid_;
                            table_name = update_log->table_name_;
                            has_valid_rid = true;
                        }
                    } else if (record->log_type_ == LogType::DELETE) {
                        DeleteLogRecord* delete_log = dynamic_cast<DeleteLogRecord*>(record);
                        if (delete_log) {
                            rid = delete_log->rid_;
                            table_name = delete_log->table_name_;
                            has_valid_rid = true;
                        }
                    }
                    
                    // 更新脏页表
                    if (has_valid_rid && rid.page_no != INVALID_PAGE_ID) {
                        if (sm_manager_->fhs_.find(table_name) != sm_manager_->fhs_.end()) {
                            PageId page_id;
                            page_id.fd = sm_manager_->fhs_[table_name]->GetFd();
                            page_id.page_no = rid.page_no;
                            
                            if (dirty_page_table_.find(page_id) == dirty_page_table_.end() ||
                                dirty_page_table_[page_id] > record->lsn_) {
                                dirty_page_table_[page_id] = record->lsn_;
                            }
                        }
                    }
                }
                break;
                
            default:
                break;
        }
    }

    std::vector<LogRecord*> logs_before_ckpt = log_manager_->scan_log_from_lsn(0, checkpoint_lsn);
    // 3. 对于active_txn_table_中仍为INVALID_LSN的事务，向前扫描日志补全
    for (auto& [txn_id, lsn] : active_txn_table_) {
        if (lsn == INVALID_LSN) {
            // 向前扫描日志，找到该事务的最后一条操作日志
            lsn_t last_lsn = INVALID_LSN;
            for (auto* record : logs_before_ckpt) {
                if (record->log_tid_ == txn_id &&
                    (record->log_type_ == LogType::INSERT ||
                    record->log_type_ == LogType::UPDATE ||
                    record->log_type_ == LogType::DELETE ||
                    record->log_type_ == LogType::begin)) {
                    last_lsn = record->lsn_;
                }
            }
            if (last_lsn != INVALID_LSN) {
                active_txn_table_[txn_id] = last_lsn;
                std::cout << "补全事务" << txn_id << " 的最后LSN为: " << last_lsn << std::endl;
            }
            // 清理
            for (auto* record : logs_before_ckpt) delete record;
        }
    }

    std::cout << "分析完成 - undo_list: " << undo_list.size() 
              << ", redo_list: " << redo_list.size() 
              << ", 脏页表大小: " << dirty_page_table_.size() << std::endl;
    
    // 🔄 第三步：先执行REDO，再执行UNDO（题目要求的步骤3）
    
    // (1) 对redo_list中的事务执行REDO
    std::cout << "开始REDO已提交事务操作，共" << redo_list.size() << "个事务" << std::endl;
    for (auto* record : log_records) {
        txn_id_t txn_id = record->log_tid_;
        if (redo_list.find(txn_id) != redo_list.end()) {
            if (record->log_type_ == LogType::INSERT ||
                record->log_type_ == LogType::UPDATE ||
                record->log_type_ == LogType::DELETE) {
                std::cout << "重做事务 " << txn_id << " 的操作, LSN=" << record->lsn_ << std::endl;
                redo_log_record(record);
            }
        }
    }
    
    // (2) 对undo_list中的事务执行UNDO
    std::cout << "开始UNDO未提交事务操作，共" << undo_list.size() << "个事务" << std::endl;
    for (auto txn_id : undo_list) {
        std::cout << "撤销事务 " << txn_id << " 的操作" << std::endl;
        if (active_txn_table_.find(txn_id) != active_txn_table_.end()) {
            std::cout << "事务 " << txn_id << " 仍然活跃，执行UNDO" << std::endl;
            lsn_t lsn = active_txn_table_[txn_id];
            if (lsn != INVALID_LSN) {
                std::cout << "事务 " << txn_id << " 的最后LSN: " << lsn << std::endl;
                // 按LSN倒序执行UNDO
                while (lsn != INVALID_LSN) {
                    LogRecord* record = log_manager_->read_log_record(lsn);
                    if (!record) break;
                    
                    if (record->log_type_ == LogType::INSERT ||
                        record->log_type_ == LogType::UPDATE ||
                        record->log_type_ == LogType::DELETE) {
                        std::cout << "撤销事务 " << txn_id << " 的操作, LSN=" << lsn << std::endl;
                        undo_log_record(record);
                    }
                    
                    lsn = record->prev_lsn_;
                    if(lsn == INVALID_LSN) {
                        std::cout << "lsn " << lsn << "是无效事务" << std::endl;
                    }
                    else {
                        std::cout << "lsn " << lsn << " 是无效事务 " << lsn << std::endl;
                    }
                    delete record;
                }
            }
        }
    }
    
    // 清理日志记录
    for (auto* record : log_records) {
        delete record;
    }
    
    // 清空表
    active_txn_table_.clear();
    dirty_page_table_.clear();
    
    std::cout << "基于检查点的恢复完成" << std::endl;

    // 扫描所有日志，找到最大LSN和最大txn_id
    lsn_t max_lsn = 0;
    txn_id_t max_txn_id = 0;
    std::vector<LogRecord*> all_log_records= log_manager_->scan_log_from_lsn(0);
    for (auto* record : all_log_records) {
        if (record->lsn_ > max_lsn) max_lsn = record->lsn_;
        if (record->log_tid_ > max_txn_id) max_txn_id = record->log_tid_;
    }
    // 恢复流程最后，设置新的起始值
    log_manager_->set_global_lsn(max_lsn + 1);
    txn_mgr_->set_next_txn_id(max_txn_id + 1);

    std::cout << "恢复后设置global_lsn=" << (max_lsn + 1)
              << "，next_txn_id=" << (max_txn_id + 1) << std::endl;

}

/**
 * @description: 重做所有未落盘的操作
 */
 void RecoveryManager::redo() {
    std::cout << "执行REDO阶段..." << std::endl;
    
    // 从最小的recLSN开始读取日志
    lsn_t min_rec_lsn = INVALID_LSN;
    for (const auto& [page_id, rec_lsn] : dirty_page_table_) {
        if (min_rec_lsn == INVALID_LSN || rec_lsn < min_rec_lsn) {
            min_rec_lsn = rec_lsn;
        }
    }
    
    // 如果没有脏页，直接返回
    if (min_rec_lsn == INVALID_LSN) {
        std::cout << "没有需要重做的操作" << std::endl;
        return;
    }
    
    // ✅ 重新构建事务状态表
    std::unordered_set<txn_id_t> committed_txns;
    std::unordered_set<txn_id_t> aborted_txns;
    
    // 扫描所有日志确定事务状态
    std::vector<LogRecord*> all_records = log_manager_->scan_log_from_lsn();
    for (auto* record : all_records) {
        if (record->log_type_ == LogType::commit) {
            committed_txns.insert(record->log_tid_);
            aborted_txns.erase(record->log_tid_);
        } else if (record->log_type_ == LogType::ABORT) {
            aborted_txns.insert(record->log_tid_);
            committed_txns.erase(record->log_tid_);
        }
    }
    
    // 清理all_records
    for (auto* record : all_records) {
        delete record;
    }
    
    std::cout << "REDO阶段事务状态统计:" << std::endl;
    std::cout << "已提交事务数: " << committed_txns.size() << std::endl;
    std::cout << "已中止事务数: " << aborted_txns.size() << std::endl;
    
    // 扫描从min_rec_lsn开始的日志
    std::vector<LogRecord*> log_records = log_manager_->scan_log_from_lsn(min_rec_lsn);
    
    // 按照LSN顺序重做操作
    for (auto* record : log_records) {
        // ✅ 关键修复：检查事务状态
        if (record->log_type_ == LogType::INSERT || 
            record->log_type_ == LogType::UPDATE || 
            record->log_type_ == LogType::DELETE) {
            
            txn_id_t txn_id = record->log_tid_;
            
            // 跳过已中止事务的操作
            if (aborted_txns.find(txn_id) != aborted_txns.end()) {
                std::cout << "REDO阶段跳过已中止事务的操作: 事务" << txn_id 
                          << ", LSN=" << record->lsn_ 
                          << ", 类型=" << static_cast<int>(record->log_type_) << std::endl;
                continue;
            }
            
            // 只重做已提交或活跃事务的操作
            bool should_redo = false;
            if (committed_txns.find(txn_id) != committed_txns.end()) {
                should_redo = true;
                std::cout << "重做已提交事务的操作: 事务" << txn_id << ", LSN=" << record->lsn_ << std::endl;
            } else if (active_txn_table_.find(txn_id) != active_txn_table_.end()) {
                should_redo = true;
                std::cout << "重做活跃事务的操作: 事务" << txn_id << ", LSN=" << record->lsn_ << std::endl;
            }
            
            if (!should_redo) {
                std::cout << "跳过未知状态事务的操作: 事务" << txn_id << ", LSN=" << record->lsn_ << std::endl;
                continue;
            }
        }
        
        Rid rid;
        bool has_valid_rid = false;
        std::string table_name; 
        
        // 根据日志类型获取rid
        if (record->log_type_ == LogType::INSERT) {
            InsertLogRecord* insert_log = dynamic_cast<InsertLogRecord*>(record);
            if (insert_log) {
                rid = insert_log->rid_;
                table_name = insert_log->table_name_;
                has_valid_rid = true;
            }
        } else if (record->log_type_ == LogType::UPDATE) {
            UpdateLogRecord* update_log = dynamic_cast<UpdateLogRecord*>(record);
            if (update_log) {
                rid = update_log->rid_;
                table_name = update_log->table_name_; 
                has_valid_rid = true;
            }
        } else if (record->log_type_ == LogType::DELETE) {
            DeleteLogRecord* delete_log = dynamic_cast<DeleteLogRecord*>(record);
            if (delete_log) {
                rid = delete_log->rid_;
                table_name = delete_log->table_name_;
                has_valid_rid = true;
            }
        }

        if (!has_valid_rid || rid.page_no == INVALID_PAGE_ID) continue;

        PageId page_id;
        page_id.fd = sm_manager_->fhs_[table_name]->GetFd();
        page_id.page_no = rid.page_no;
        
        // 检查是否需要重做（页面在脏页表中且日志LSN >= recLSN）
        if (dirty_page_table_.find(page_id) != dirty_page_table_.end() &&
            record->lsn_ >= dirty_page_table_[page_id]) {
            
            // 检查页面在缓冲池中的LSN是否小于日志LSN
            Page* page = buffer_pool_manager_->fetch_page(page_id);
            if (page && page->get_page_lsn() < record->lsn_) {
                // 重做操作
                std::cout << "执行REDO: 事务" << record->log_tid_ 
                          << ", LSN=" << record->lsn_ 
                          << ", 类型=" << static_cast<int>(record->log_type_) << std::endl;
                redo_log_record(record);
                // 更新页面LSN
                page->set_page_lsn(record->lsn_);
            }
            if (page) buffer_pool_manager_->unpin_page(page_id, true);
        }
    }
    
    // 清理日志记录
    for (auto* record : log_records) {
        delete record;
    }
    std::cout << "REDO阶段完成" << std::endl;
}

/**
 * @description: 回滚未完成的事务
 */
void RecoveryManager::undo() {
        std::cout << "执行UNDO阶段..." << std::endl;

        // 如果没有未完成的事务，直接返回
        if (active_txn_table_.empty()) {
            std::cout << "没有需要撤销的事务" << std::endl;
            return;
        }
        // 构建每个事务的最后一个LSN映射
        std::unordered_map<txn_id_t, lsn_t> txn_last_lsn_map = active_txn_table_;
        // 按照LSN从大到小的顺序回滚操作
        while (!txn_last_lsn_map.empty()) {
            // 找到最大的LSN及其对应的事务
            txn_id_t max_txn_id = INVALID_TXN_ID;
            lsn_t max_lsn = INVALID_LSN;
            
            for (const auto& [txn_id, lsn] : txn_last_lsn_map) {
                if (lsn != INVALID_LSN && (max_lsn == INVALID_LSN || lsn > max_lsn)) {
                    max_txn_id = txn_id;
                    max_lsn = lsn;
                }
            }
            
            // 如果没有找到有效的LSN，退出循环
            if (max_txn_id == INVALID_TXN_ID) break;
            
            // 读取对应的日志记录
            LogRecord* record = log_manager_->read_log_record(max_lsn);
            if (!record) {
                // 如果找不到日志记录，从映射中移除该事务
                txn_last_lsn_map.erase(max_txn_id);
                continue;
            }
            
            // 撤销操作
            switch (record->log_type_) {
                case LogType::UPDATE:
                case LogType::INSERT:
                case LogType::DELETE:
                    undo_log_record(record);
                    break;
                    
                default:
                    // 忽略其他类型的日志记录
                    break;
            }
            
            // 更新事务的LSN为前一个LSN
            lsn_t prev_lsn = record->prev_lsn_;
            if (prev_lsn == INVALID_LSN) {
                // 如果没有前一个LSN，说明已回滚到事务的开始，从映射中移除
                txn_last_lsn_map.erase(max_txn_id);
            } else {
                // 否则更新为前一个LSN
                txn_last_lsn_map[max_txn_id] = prev_lsn;
            }
            
            delete record;
        }
        
        std::cout << "UNDO阶段完成" << std::endl;
}

/**
 * @description: 重做单个日志记录
 * @param {LogRecord*} log_record 日志记录
 */
 void RecoveryManager::redo_log_record(LogRecord* log_record) {
    if (!log_record) return;
    
    try {
        switch (log_record->log_type_) {
            case LogType::INSERT: {
                InsertLogRecord* insert_log = dynamic_cast<InsertLogRecord*>(log_record);
                if (insert_log) {
                    // 获取表和RID信息
                    std::string table_name = insert_log->table_name_;
                    Rid rid = insert_log->rid_;
                    
                    // 检查表是否存在
                    if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
                        std::cerr << "表不存在: " << table_name << std::endl;
                        return;
                    }
                    
                    RmFileHandle* file_handle = sm_manager_->fhs_[table_name].get();
                    if (file_handle) {
                        // 重做插入记录
                        file_handle->insert_record(rid, insert_log->insert_value_.data);
                        
                        // 🔄 更新索引
                        TabMeta& tab_meta = sm_manager_->db_.get_table(table_name);
                        for (auto& index : tab_meta.indexes) {
                            std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
                            auto ih = sm_manager_->ihs_.at(index_name).get();
                            
                            // 构建索引键
                            char* key = new char[index.col_tot_len];
                            int offset = 0;
                            for (size_t i = 0; i < index.cols.size(); i++) {
                                memcpy(key + offset, 
                                       insert_log->insert_value_.data + index.cols[i].offset, 
                                       index.cols[i].len);
                                offset += index.cols[i].len;
                            }
                            
                            // 插入索引项
                            ih->insert_entry(key, rid, nullptr);
                            delete[] key;
                        }
                        
                        std::cout << "重做INSERT: 表=" << table_name 
                                  << ", RID=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                    }
                }
                break;
            }
            
            case LogType::DELETE: {
                DeleteLogRecord* delete_log = dynamic_cast<DeleteLogRecord*>(log_record);
                if (delete_log) {
                    // 获取表和RID信息
                    std::string table_name = delete_log->table_name_;
                    Rid rid = delete_log->rid_;
                    
                    // 检查表是否存在
                    if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
                        std::cerr << "表不存在: " << table_name << std::endl;
                        return;
                    }
                    
                    RmFileHandle* file_handle = sm_manager_->fhs_[table_name].get();
                    if (file_handle) {
                        // 🔄 先删除索引
                        TabMeta& tab_meta = sm_manager_->db_.get_table(table_name);
                        for (auto& index : tab_meta.indexes) {
                            std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
                            auto ih = sm_manager_->ihs_.at(index_name).get();
                            
                            // 构建索引键
                            char* key = new char[index.col_tot_len];
                            int offset = 0;
                            for (size_t i = 0; i < index.cols.size(); i++) {
                                memcpy(key + offset, 
                                       delete_log->deleted_value_.data + index.cols[i].offset, 
                                       index.cols[i].len);
                                offset += index.cols[i].len;
                            }
                            
                            // 删除索引项
                            ih->delete_entry(key, nullptr);
                            delete[] key;
                        }
                        
                        // 删除记录
                        file_handle->delete_record(rid, nullptr);
                        std::cout << "重做DELETE: 表=" << table_name 
                                  << ", RID=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                    }
                }
                break;
            }
            
            case LogType::UPDATE: {
                UpdateLogRecord* update_log = dynamic_cast<UpdateLogRecord*>(log_record);
                if (update_log) {
                    // 获取表和RID信息
                    std::string table_name = update_log->table_name_;
                    Rid rid = update_log->rid_;
                    
                    // 检查表是否存在
                    if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
                        std::cerr << "表不存在: " << table_name << std::endl;
                        return;
                    }
                    
                    RmFileHandle* file_handle = sm_manager_->fhs_[table_name].get();
                    if (file_handle) {
                        // 🔄 更新索引
                        TabMeta& tab_meta = sm_manager_->db_.get_table(table_name);
                        for (auto& index : tab_meta.indexes) {
                            std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
                            auto ih = sm_manager_->ihs_.at(index_name).get();
                            
                            // 构建旧索引键
                            char* old_key = new char[index.col_tot_len];
                            int offset = 0;
                            for (size_t i = 0; i < index.cols.size(); i++) {
                                memcpy(old_key + offset, 
                                       update_log->old_value_.data + index.cols[i].offset, 
                                       index.cols[i].len);
                                offset += index.cols[i].len;
                            }
                            
                            // 删除旧索引
                            ih->delete_entry(old_key, nullptr);
                            delete[] old_key;
                            
                            // 构建新索引键
                            char* new_key = new char[index.col_tot_len];
                            offset = 0;
                            for (size_t i = 0; i < index.cols.size(); i++) {
                                memcpy(new_key + offset, 
                                       update_log->new_value_.data + index.cols[i].offset, 
                                       index.cols[i].len);
                                offset += index.cols[i].len;
                            }
                            
                            // 插入新索引
                            ih->insert_entry(new_key, rid, nullptr);
                            delete[] new_key;
                        }
                        
                        // 更新记录
                        file_handle->update_record(rid, update_log->new_value_.data, nullptr);
                        std::cout << "重做UPDATE: 表=" << table_name 
                                  << ", RID=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                    }
                }
                break;
            }
            
            default:
                // 忽略其他类型的日志记录
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "重做操作失败: " << e.what() << std::endl;
    }
}
    
/**
    * @description: 撤销单个日志记录
    * @param {LogRecord*} log_record 日志记录
    */
void RecoveryManager::undo_log_record(LogRecord* log_record) {
if (!log_record) return;

try {
    switch (log_record->log_type_) {
        case LogType::INSERT: {
            InsertLogRecord* insert_log = dynamic_cast<InsertLogRecord*>(log_record);
            if (insert_log) {
                // 获取表和RID信息
                std::string table_name = insert_log->table_name_;
                Rid rid = insert_log->rid_;
                
                // 检查表是否存在
                if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
                    std::cerr << "表不存在: " << table_name << std::endl;
                    return;
                }
                
                RmFileHandle* file_handle = sm_manager_->fhs_[table_name].get();
                if (file_handle) {
                    // 🔄 先删除索引
                    TabMeta& tab_meta = sm_manager_->db_.get_table(table_name);
                    for (auto& index : tab_meta.indexes) {
                        std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
                        auto ih = sm_manager_->ihs_.at(index_name).get();
                        
                        // 构建索引键
                        char* key = new char[index.col_tot_len];
                        int offset = 0;
                        for (size_t i = 0; i < index.cols.size(); i++) {
                            memcpy(key + offset, 
                                   insert_log->insert_value_.data + index.cols[i].offset, 
                                   index.cols[i].len);
                            offset += index.cols[i].len;
                        }
                        
                        // 删除索引项
                        ih->delete_entry(key, nullptr);
                        delete[] key;
                    }
                    
                    // 删除记录
                    try {
                        // 删除记录
                        file_handle->delete_record(rid, nullptr);
                        std::cout << "撤销INSERT: 表=" << table_name 
                                  << ", RID=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                    } catch (const RecordNotFoundError& e) {
                        // 记录不存在是正常情况 - 可能数据页未刷盘就崩溃了
                        std::cout << "撤销INSERT: 记录已不存在，无需删除: 表=" << table_name 
                                  << ", RID=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                    } catch (const std::exception& e) {
                        // 其他错误记录但不中断恢复过程
                        std::cerr << "撤销INSERT失败(非致命错误): " << e.what() << std::endl;
                    }
                }
            }
            break;
        }
        
        case LogType::DELETE: {
            DeleteLogRecord* delete_log = dynamic_cast<DeleteLogRecord*>(log_record);
            if (delete_log) {
                // 获取表和RID信息
                std::string table_name = delete_log->table_name_;
                Rid rid = delete_log->rid_;
                
                // 检查表是否存在
                if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
                    std::cerr << "表不存在: " << table_name << std::endl;
                    return;
                }
                
                RmFileHandle* file_handle = sm_manager_->fhs_[table_name].get();
                if (file_handle) {
                    // 插入记录
                    file_handle->insert_record(rid, delete_log->deleted_value_.data);
                    
                    // 🔄 插入索引
                    TabMeta& tab_meta = sm_manager_->db_.get_table(table_name);
                    for (auto& index : tab_meta.indexes) {
                        std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
                        auto ih = sm_manager_->ihs_.at(index_name).get();
                        
                        // 构建索引键
                        char* key = new char[index.col_tot_len];
                        int offset = 0;
                        for (size_t i = 0; i < index.cols.size(); i++) {
                            memcpy(key + offset, 
                                   delete_log->deleted_value_.data + index.cols[i].offset, 
                                   index.cols[i].len);
                            offset += index.cols[i].len;
                        }
                        
                        // 插入索引项
                        ih->insert_entry(key, rid, nullptr);
                        delete[] key;
                    }
                    
                    std::cout << "撤销DELETE: 表=" << table_name 
                                << ", RID=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                }
            }
            break;
        }
        
        case LogType::UPDATE: {
            UpdateLogRecord* update_log = dynamic_cast<UpdateLogRecord*>(log_record);
            if (update_log) {
                // 获取表和RID信息
                std::string table_name = update_log->table_name_;
                Rid rid = update_log->rid_;
                
                // 检查表是否存在
                if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
                    std::cerr << "表不存在: " << table_name << std::endl;
                    return;
                }
                
                RmFileHandle* file_handle = sm_manager_->fhs_[table_name].get();
                if (file_handle) {
                    // 🔄 更新索引
                    TabMeta& tab_meta = sm_manager_->db_.get_table(table_name);
                    for (auto& index : tab_meta.indexes) {
                        std::string index_name = sm_manager_->get_ix_manager()->get_index_name(table_name, index.cols);
                        auto ih = sm_manager_->ihs_.at(index_name).get();
                        
                        // 构建旧索引键
                        char* new_key = new char[index.col_tot_len];
                        int offset = 0;
                        for (size_t i = 0; i < index.cols.size(); i++) {
                            memcpy(new_key + offset, 
                                   update_log->new_value_.data + index.cols[i].offset, 
                                   index.cols[i].len);
                            offset += index.cols[i].len;
                        }
                        
                        // 删除新索引
                        ih->delete_entry(new_key, nullptr);
                        delete[] new_key;
                        
                        // 构建旧索引键
                        char* old_key = new char[index.col_tot_len];
                        offset = 0;
                        for (size_t i = 0; i < index.cols.size(); i++) {
                            memcpy(old_key + offset, 
                                   update_log->old_value_.data + index.cols[i].offset, 
                                   index.cols[i].len);
                            offset += index.cols[i].len;
                        }
                        
                        // 插入旧索引
                        ih->insert_entry(old_key, rid, nullptr);
                        delete[] old_key;
                    }
                    
                    // 恢复旧记录
                    file_handle->update_record(rid, update_log->old_value_.data, nullptr);
                    std::cout << "撤销UPDATE: 表=" << table_name 
                                << ", RID=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                }
            }
            break;
        }
        
        default:
            // 忽略其他类型的日志记录
            break;
    }
} catch (const std::exception& e) {
    std::cerr << "撤销操作失败: " << e.what() << std::endl;
}
}

LogRecord* RecoveryManager::create_log_record_by_type(LogType type) {
    printf("DEBUG: 创建日志记录，类型=%d\n", (int)type);
    
    // 添加类型安全检查
    if ((int)type < 0 || (int)type > 6) {
        printf("警告: 无效的日志类型 %d，使用默认类型(UPDATE)\n", (int)type);
        return new UpdateLogRecord(); // 返回默认记录而不是抛出异常
    }
    
    switch (type) {
        case LogType::begin:
            return new BeginLogRecord();
        case LogType::commit:
            return new CommitLogRecord();
        case LogType::ABORT:
            return new AbortLogRecord();
        case LogType::INSERT:
            return new InsertLogRecord();
        case LogType::UPDATE:
            return new UpdateLogRecord();
        case LogType::DELETE:
            return new DeleteLogRecord();
        case LogType::CHECKPOINT:
            return new CheckpointLogRecord();
        default:
            printf("警告: 未知的日志类型 %d，使用默认类型\n", (int)type);
            return new UpdateLogRecord();
    }
}

