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
    std::vector<LogRecord*> log_records;
    if (checkpoint_lsn == INVALID_LSN) {
        log_records = log_manager_->scan_log_from_lsn();  // 从头开始
    } else {
        log_records = log_manager_->scan_log_from_lsn(checkpoint_lsn);  // 从检查点开始
    }
    
    std::cout << "从日志中读取了 " << log_records.size() << " 条记录" << std::endl;
    
    // ✅ 修复：收集事务最终状态
    std::unordered_set<txn_id_t> committed_txns;
    std::unordered_set<txn_id_t> aborted_txns;
    
    // 第一遍扫描：确定事务状态和构建活跃事务表
    for (auto* record : log_records) {
        std::cout << "日志记录: 类型=" << static_cast<int>(record->log_type_) 
                  << ", 事务ID=" << record->log_tid_ 
                  << ", LSN=" << record->lsn_ << std::endl;
        
        txn_id_t txn_id = record->log_tid_;
        
        switch (record->log_type_) {
            case LogType::begin:
                active_txn_table_[txn_id] = record->lsn_;
                std::cout << "发现开始事务: " << txn_id << std::endl;
                break;
                
            case LogType::commit:
                active_txn_table_.erase(txn_id);
                committed_txns.insert(txn_id);
                aborted_txns.erase(txn_id);  // 防止状态冲突
                std::cout << "发现已提交事务: " << txn_id << std::endl;
                break;
                
            case LogType::ABORT:
                active_txn_table_.erase(txn_id);
                aborted_txns.insert(txn_id);
                committed_txns.erase(txn_id);  // 防止状态冲突
                std::cout << "发现已中止事务: " << txn_id << std::endl;
                break;
                
            case LogType::INSERT:
            case LogType::UPDATE:
            case LogType::DELETE:
                // 更新活跃事务的最新LSN
                if (active_txn_table_.find(txn_id) != active_txn_table_.end()) {
                    active_txn_table_[txn_id] = record->lsn_;
                }
                std::cout << "处理数据修改日志: 类型=" << static_cast<int>(record->log_type_) 
                          << ", 事务ID=" << txn_id 
                          << ", LSN=" << record->lsn_ << std::endl;
                break;
                
            default:
                break;
        }
    }
    
    // ✅ 第二遍扫描：只为已提交或活跃事务构建脏页表
    for (auto* record : log_records) {
        if (record->log_type_ == LogType::INSERT || 
            record->log_type_ == LogType::UPDATE || 
            record->log_type_ == LogType::DELETE) {
            
            txn_id_t txn_id = record->log_tid_;
            
            // ✅ 关键修复：检查事务状态
            bool should_add_to_dirty_pages = false;
            
            if (committed_txns.find(txn_id) != committed_txns.end()) {
                // 已提交事务，需要重做
                should_add_to_dirty_pages = true;
                std::cout << "已提交事务的操作: 事务" << txn_id << ", LSN=" << record->lsn_ << std::endl;
            } else if (active_txn_table_.find(txn_id) != active_txn_table_.end()) {
                // 仍然活跃的事务，也需要重做（稍后会被UNDO）
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
}

/**
 * @description: 从最后一个检查点恢复
 */
 void RecoveryManager::recover_from_checkpoint() {
        std::cout << "从检查点开始恢复..." << std::endl;
        
        // 获取最后一个检查点的LSN
        lsn_t checkpoint_lsn = log_manager_->get_last_checkpoint_lsn();
        if (checkpoint_lsn == INVALID_LSN) {
            throw std::runtime_error("无效的检查点LSN");
        }
        
        // 读取检查点记录
        LogRecord* checkpoint_record = nullptr;
        try {
            checkpoint_record = log_manager_->read_log_record(checkpoint_lsn);
            if (checkpoint_record->log_type_ != LogType::CHECKPOINT) {
                throw std::runtime_error("LSN对应的记录不是检查点记录");
            }
            
            // 从检查点获取活跃事务列表
            CheckpointLogRecord* checkpoint = dynamic_cast<CheckpointLogRecord*>(checkpoint_record);
            active_txn_table_.clear();
            // 获取活跃事务的方法名
            std::vector<txn_id_t> active_txns = checkpoint->get_active_txns();
            // 将检查点时的活跃事务加入ATT
            for (auto txn_id : active_txns) {
                active_txn_table_[txn_id] = INVALID_LSN; // 初始化为无效LSN，稍后会更新
                std::cout << "从检查点添加活跃事务: " << txn_id << std::endl;
            }
            
            delete checkpoint_record;
        } catch (const std::exception& e) {
            if (checkpoint_record) delete checkpoint_record;
            std::cerr << "读取检查点记录失败: " << e.what() << std::endl;
            throw;
        }
        
        // 从检查点开始扫描日志
        std::vector<LogRecord*> log_records = log_manager_->scan_log_from_lsn(checkpoint_lsn);
        
        // redo_list记录需要重做的事务，初始为空
        std::unordered_set<txn_id_t> redo_list;
        
        // 分析日志记录
        for (auto* record : log_records) {
            txn_id_t txn_id = record->log_tid_;
            
            switch (record->log_type_) {
                case LogType::begin:
                    // 新事务开始，加入ATT
                    active_txn_table_[txn_id] = record->lsn_;
                    break;
                case LogType::commit:
                    // 事务提交，从ATT移除，加入redo_list
                    active_txn_table_.erase(txn_id);
                    redo_list.insert(txn_id);
                    break;   
                case LogType::ABORT:
                    // 事务中止，从ATT移除
                    active_txn_table_.erase(txn_id);
                    break;
                case LogType::UPDATE:
                case LogType::INSERT:
                case LogType::DELETE:
                // 更新ATT中的LSN
                if (active_txn_table_.find(txn_id) != active_txn_table_.end()) {
                        active_txn_table_[txn_id] = record->lsn_;
                }      
                // 更新脏页表
                {  
                        Rid rid;
                        bool has_valid_rid = false;
                        std::string table_name;
                        // 根据日志类型获取对应的rid
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
                        
                        if (has_valid_rid && rid.page_no != INVALID_PAGE_ID) {
                        // 检查表是否存在
                        if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
                                std::cerr << "警告: 表 " << table_name << " 不存在或未打开" << std::endl;
                                continue;
                        }
                        PageId page_id;
                        page_id.fd = sm_manager_->fhs_[table_name]->GetFd();  // 获取文件描述符
                        page_id.page_no = rid.page_no;
                        
                        if (dirty_page_table_.find(page_id) == dirty_page_table_.end() ||
                                dirty_page_table_[page_id] > record->lsn_) {
                                dirty_page_table_[page_id] = record->lsn_;
                        }
                        }
                }
                break;
                    
                default:
                    // 忽略其他类型的日志记录
                    break;
            }
        }
        std::cout << "开始REDO阶段，需要重做的事务数: " << redo_list.size() << std::endl;
        redo();
        std::cout << "开始UNDO阶段，需要撤销的事务数: " << active_txn_table_.size() << std::endl;
        undo();
        // 清理日志记录
        for (auto* record : log_records) {
            delete record;
        }
        // 恢复完成后清空脏页表和活跃事务表
        active_txn_table_.clear();
        dirty_page_table_.clear();
        //truncate_log_after_recovery();//日志截断
        std::cout << "从检查点恢复完成" << std::endl;
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
        
        // 原来的重做逻辑
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
                        std::string table_name = insert_log->table_name_; // 直接使用成员变量
                        Rid rid = insert_log->rid_;  
                        RmFileHandle* file_handle = sm_manager_->fhs_[table_name].get();
                        if (file_handle) {
                            file_handle->insert_record(rid, insert_log->insert_value_.data);
                            std::cout << "重做INSERT: table=" << table_name << std::endl;
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
                        RmFileHandle* file_handle = sm_manager_->fhs_[table_name].get();
                        if (file_handle) {
                            file_handle->update_record(rid, update_log->new_value_.data, nullptr);
                            std::cout << "重做UPDATE: table=" << table_name 
                                << ", rid=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
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
                        RmFileHandle* file_handle = sm_manager_->fhs_[table_name].get();
                        if (file_handle) {
                            file_handle->delete_record(rid, nullptr);
                            std::cout << "重做DELETE: table=" << table_name 
                                << ", rid=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
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
            // 在处理前验证日志类型有效性
            if (static_cast<int>(log_record->log_type_) < 0 || 
                static_cast<int>(log_record->log_type_) > static_cast<int>(LogType::CHECKPOINT)) {
                std::cerr << "错误: UNDO阶段遇到无效的日志类型: " << static_cast<int>(log_record->log_type_) << std::endl;
                return;  // 跳过此记录
            }
            
            switch (log_record->log_type_) {
                case LogType::INSERT: {
                    InsertLogRecord* insert_record = dynamic_cast<InsertLogRecord*>(log_record);
                    std::string tab_name(insert_record->table_name_);
                    auto fh = sm_manager_->fhs_.at(tab_name).get();
                    fh->delete_record(insert_record->rid_, nullptr);
                    std::cout << "撤销INSERT: table=" << tab_name << ", rid=(" 
                            << insert_record->rid_.page_no << "," << insert_record->rid_.slot_no << ")" << std::endl;
                    break;
                }
                case LogType::DELETE: {
                    DeleteLogRecord* delete_record = dynamic_cast<DeleteLogRecord*>(log_record);
                    std::string tab_name(delete_record->table_name_);
                    auto fh = sm_manager_->fhs_.at(tab_name).get();
                    fh->insert_record(delete_record->rid_, delete_record->deleted_value_.data);
                    std::cout << "撤销DELETE: table=" << tab_name << ", rid=(" 
                            << delete_record->rid_.page_no << "," << delete_record->rid_.slot_no << ")" << std::endl;
                    break;
                }
                case LogType::UPDATE: {
                    UpdateLogRecord* update_record = dynamic_cast<UpdateLogRecord*>(log_record);
                    std::string tab_name(update_record->table_name_);
                    auto fh = sm_manager_->fhs_.at(tab_name).get();
                    
                    // 表达式更新与普通更新的UNDO操作相同，都是恢复旧记录
                    fh->update_record(update_record->rid_, update_record->old_value_.data, nullptr);
                    
                    std::cout << "撤销UPDATE: table=" << tab_name << ", rid=(" 
                            << update_record->rid_.page_no << "," << update_record->rid_.slot_no << ")";
                    
                    // 添加表达式信息输出（帮助调试）
                    if (update_record->is_expr_update_) {
                        std::cout << " [表达式更新: " << update_record->op_type_ << "]";
                    }
                    std::cout << std::endl;
                    
                    break;
                }
            default:
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "撤销操作失败: " << e.what() << std::endl;
    }
    }

/**
 * @brief 执行日志截断，在恢复完成后调用
 * @param checkpoint_interval 两次检查点之间的间隔（毫秒）
 */
void RecoveryManager::truncate_log_after_recovery() {
        // 获取当前系统的最大LSN
        if (log_manager_ == nullptr) {
                std::cerr << "错误: log_manager_为空，无法执行日志截断" << std::endl;
                return;}
        std::cout<<""<< "开始执行日志截断..." << std::endl;
        lsn_t current_lsn = log_manager_->get_current_lsn();
        std::vector<txn_id_t> active_txns;
        for (const auto& pair : active_txn_table_) {
            active_txns.push_back(pair.first);
        }
        // 添加检查点记录
        std::cout << "创建检查点记录..." << std::endl;
        lsn_t checkpoint_lsn = log_manager_->create_checkpoint(active_txns);
        if (checkpoint_lsn == INVALID_LSN) {
            std::cerr << "创建检查点失败，不执行日志截断" << std::endl;
            return;
        }
        std::cout << "已创建新检查点，LSN: " << checkpoint_lsn << std::endl;
        // 获取最小LSN，确保不会截断任何可能需要的日志
        lsn_t min_lsn = INVALID_LSN;
        // 如果脏页表非空，获取最小的LSN
        for (const auto& entry : dirty_page_table_) {
            if (min_lsn == INVALID_LSN || entry.second < min_lsn) {
                min_lsn = entry.second;
            }
        }
        // 如果活跃事务表非空，找到最早的LSN
        for (const auto& entry : active_txn_table_) {
            if (min_lsn == INVALID_LSN || entry.second < min_lsn) {
                min_lsn = entry.second;
            }
        }
        // 如果没有找到最小LSN，使用检查点LSN作为截断点
        lsn_t truncate_lsn = (min_lsn == INVALID_LSN) ? checkpoint_lsn : std::min(min_lsn, checkpoint_lsn);
        // 执行截断操作
        std::cout << "开始截断日志: " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
        bool success = log_manager_->truncate_log(truncate_lsn);
        std::cout << "完成截断日志: " << std::chrono::system_clock::now().time_since_epoch().count() << std::endl;
        
        if (success) {
            std::cout << "日志截断成功，截断点LSN: " << truncate_lsn << std::endl;
        } else {
            std::cerr << "日志截断失败" << std::endl;
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