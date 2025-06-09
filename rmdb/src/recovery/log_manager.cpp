/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include <cstring>
#include "log_manager.h"

/**
 * @description: 添加日志记录到日志缓冲区中，并返回日志记录号
 * @param {LogRecord*} log_record 要写入缓冲区的日志记录
 * @return {lsn_t} 返回该日志的日志记录号
 */
lsn_t LogManager::add_log_to_buffer(LogRecord* log_record) {
        std::unique_lock<std::mutex> latch(latch_);
    
        // 为日志记录分配一个LSN
        lsn_t lsn = global_lsn_++;
        log_record->lsn_ = lsn;
        
        // 检查缓冲区是否有足够空间
        if (log_buffer_.is_full(log_record->log_tot_len_)) {
            flush_log_to_disk();  // 如果满了，先刷新到磁盘
        }
        
        // 序列化日志记录到缓冲区
        char* buffer_content = log_buffer_.buffer_ + log_buffer_.offset_;//定位起始位置
        log_record->serialize(buffer_content);
        log_buffer_.offset_ += log_record->log_tot_len_;//更新偏移量
        
        return lsn;
}

/**
 * @description: 把日志缓冲区的内容刷到磁盘中，由于目前只设置了一个缓冲区，因此需要阻塞其他日志操作
 */
void LogManager::flush_log_to_disk() {
        std::unique_lock<std::mutex> latch(latch_);//上锁

        // 仅当缓冲区有内容时才写入
        if (log_buffer_.offset_ > 0) {
                try {
                // 确保日志文件存在
                if (!disk_manager_->is_file(LOG_FILE_NAME)) {
                        disk_manager_->create_file(LOG_FILE_NAME);
                        std::cout << "Created log file: " << LOG_FILE_NAME << std::endl;
                }
                //写入磁盘
                disk_manager_->write_log(log_buffer_.buffer_, log_buffer_.offset_);
                persist_lsn_ = *reinterpret_cast<lsn_t*>(log_buffer_.buffer_ + OFFSET_LSN);
                
                // 重置日志缓冲区
                log_buffer_.offset_ = 0;
                memset(log_buffer_.buffer_, 0, sizeof(log_buffer_.buffer_));
                } catch (const std::exception& e) {
                std::cerr << "Error writing log to disk: " << e.what() << std::endl;
                throw; 
                }
        }
}
