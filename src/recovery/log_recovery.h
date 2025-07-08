/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include <map>
#include <unordered_map>
#include "log_manager.h"
#include "storage/disk_manager.h"
#include "system/sm_manager.h"

class RedoLogsInPage {
public:
    RedoLogsInPage() { table_file_ = nullptr; }
    RmFileHandle* table_file_;
    std::vector<lsn_t> redo_logs_;   // 在该page上需要redo的操作的lsn
};

class RecoveryManager {
public:
    RecoveryManager(DiskManager* disk_manager, BufferPoolManager* buffer_pool_manager, SmManager* sm_manager
                    , LogManager* log_manager) {
        disk_manager_ = disk_manager;
        buffer_pool_manager_ = buffer_pool_manager;
        sm_manager_ = sm_manager;
        log_manager_ = log_manager;
    }
    // 执行基于检查点的恢复
    void recover_from_checkpoint();
    
    // 从磁盘加载日志记录
    std::vector<LogRecord*> load_log_records(lsn_t start_lsn = INVALID_LSN);
    
    // 根据类型创建日志记录对象
    LogRecord* create_log_record_by_type(LogType type);
    void build_dirty_page_table_for_record(LogRecord* record);
    void analyze();
    void redo();
    void undo();
    void truncate_log_after_recovery();
private:
    // 活跃事务表
    std::unordered_map<txn_id_t, lsn_t> active_txn_table_;
    // 脏页表
    std::unordered_map<PageId, lsn_t> dirty_page_table_;
    LogBuffer buffer_;                                              // 读入日志
    DiskManager* disk_manager_;                                     // 用来读写文件
    BufferPoolManager* buffer_pool_manager_;                        // 对页面进行读写
    SmManager* sm_manager_;        
    LogManager* log_manager_;  // 日志管理器   
    // 重做单个日志记录
    void redo_log_record(LogRecord* log_record);
    
    // 撤销单个日志记录
    void undo_log_record(LogRecord* log_record);
};