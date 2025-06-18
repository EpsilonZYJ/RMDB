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
        // 获取所有表名
        std::vector<std::string> tables;
        sm_manager_->get_all_tables(tables);
        
        for (const auto& table_name : tables) {
            // 尝试打开表文件
            if (sm_manager_->fhs_.find(table_name) == sm_manager_->fhs_.end()) {
                try {
                    sm_manager_->fhs_[table_name] = sm_manager_->open_table_file(table_name);
                    std::cout << "恢复前打开表: " << table_name << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "打开表失败: " << table_name << " - " << e.what() << std::endl;
                }
            }
        }
        
        // 获取最后一个检查点的LSN
        lsn_t checkpoint_lsn = log_manager_->get_last_checkpoint_lsn();
        
        if (checkpoint_lsn != INVALID_LSN) {
            std::cout << "找到检查点，LSN: " << checkpoint_lsn << std::endl;
            recover_from_checkpoint();
        } else {
            std::cout << "未找到检查点，将从头开始恢复" << std::endl;
            
            // 从头扫描日志以识别活跃事务和脏页
            active_txn_table_.clear();
            dirty_page_table_.clear();
            
            // 从头获取所有日志记录
            std::vector<LogRecord*> log_records = log_manager_->scan_log_from_lsn();
            std::cout << "从日志中读取了 " << log_records.size() << " 条记录" << std::endl;
            
            // 构建活跃事务表和脏页表
            std::unordered_set<txn_id_t> committed_txns;
            
            for (auto* record : log_records) {
                txn_id_t txn_id = record->log_tid_;
                std::cout << "日志记录: 类型=" << static_cast<int>(record->log_type_) 
              << ", 事务ID=" << txn_id
              << ", LSN=" << record->lsn_ << std::endl;
                // 处理不同类型的日志记录
                switch (record->log_type_) {
                    case LogType::begin:
                        // 事务开始，添加到活跃事务表
                        active_txn_table_[txn_id] = record->lsn_;
                        std::cout << "发现开始事务: " << txn_id << std::endl;
                        break;
                        
                    case LogType::commit:
                        // 事务提交，从活跃事务表中移除
                        active_txn_table_.erase(txn_id);
                        committed_txns.insert(txn_id);
                        std::cout << "发现已提交事务: " << txn_id << std::endl;
                        break;
                        
                    case LogType::ABORT:
                        // 事务中止，从活跃事务表中移除
                        active_txn_table_.erase(txn_id);
                        std::cout << "发现已中止事务: " << txn_id << std::endl;
                        break;
                        
                    case LogType::INSERT:
                    case LogType::UPDATE:
                    case LogType::DELETE:
                        // 更新活跃事务表中的LSN
                        if (committed_txns.find(txn_id) == committed_txns.end() && 
                            active_txn_table_.find(txn_id) != active_txn_table_.end()) {
                            active_txn_table_[txn_id] = record->lsn_;
                        }
                        
                        // 更新脏页表 - 注意：这里移出了条件判断
                        {
                            Rid rid;
                            bool has_valid_rid = false;
                            std::string table_name;
    
                            std::cout << "处理数据修改日志: 类型=" << static_cast<int>(record->log_type_) 
                                    << ", 事务ID=" << txn_id
                                    << ", LSN=" << record->lsn_ << std::endl;
                            
                            // 根据日志类型获取对应的rid和表名
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
                                
                                // 更新脏页表
                                if (dirty_page_table_.find(page_id) == dirty_page_table_.end() ||
                                    dirty_page_table_[page_id] > record->lsn_) {
                                    dirty_page_table_[page_id] = record->lsn_;
                                    std::cout << "添加脏页: 表=" << table_name 
                                        << ", 页号=" << rid.page_no 
                                        << ", LSN=" << record->lsn_ << std::endl;
                                }
                            }
                        }
                        break;
                        
                    default:
                        // 忽略其他类型的日志记录
                        break;
                }
            }
            
            // 输出分析结果
            std::cout << "活跃事务表大小: " << active_txn_table_.size() << std::endl;
            std::cout << "脏页表大小: " << dirty_page_table_.size() << std::endl;
            
            // 清理日志记录
            for (auto* record : log_records) {
                delete record;
            }
            
            // 执行redo和undo操作
            redo();
            undo();

            // 清空脏页表和活跃事务表
            active_txn_table_.clear();
            dirty_page_table_.clear();
            //truncate_log_after_recovery();//日志截断
        }
        
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
    // 扫描日志
    std::vector<LogRecord*> log_records = log_manager_->scan_log_from_lsn(min_rec_lsn);
    // 按照LSN顺序重做操作
    for (auto* record : log_records) {
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
        page_id.fd = sm_manager_->fhs_[table_name]->GetFd();  // 获取文件描述符
        page_id.page_no = rid.page_no;
        // 比较 PageId 和 INVALID_PAGE_ID
        if (rid.page_no == INVALID_PAGE_ID) continue;
        // 检查是否需要重做（页面在脏页表中且日志LSN >= recLSN）
        if (dirty_page_table_.find(page_id) != dirty_page_table_.end() &&
        record->lsn_ >= dirty_page_table_[page_id]) {
            // 检查页面在缓冲池中的LSN是否小于日志LSN
            Page* page = buffer_pool_manager_->fetch_page(page_id);
            if (page && page->get_page_lsn() < record->lsn_) {
                // 重做操作
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
        switch (log_record->log_type_) {
            case LogType::INSERT: {
                InsertLogRecord* insert_log = dynamic_cast<InsertLogRecord*>(log_record);
                if (insert_log) {
                    // 获取表和RID信息
                    std::string table_name = insert_log->table_name_;
                    Rid rid = insert_log->rid_;
                    RmFileHandle* file_handle = sm_manager_->fhs_[table_name].get();
                    if (file_handle) {
                        file_handle->delete_record(rid, nullptr);
                        std::cout << "撤销INSERT: table=" << table_name 
                        << ", rid=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
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
                        file_handle->update_record(rid, update_log->old_value_.data, nullptr);
                        std::cout << "撤销UPDATE: table=" << table_name 
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
                        file_handle->insert_record(rid, delete_log->deleted_value_.data);
                        std::cout << "撤销DELETE: table=" << table_name 
                        << ", rid=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                    }
                }
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