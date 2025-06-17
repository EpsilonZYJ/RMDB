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
#include "system/sm_manager.h"
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

/**
 * @description: 创建静态检查点
 * @param {TransactionManager*} txn_manager 事务管理器
 * @param {BufferPoolManager*} buffer_pool_manager 缓冲池管理器
 */
 void LogManager::create_static_checkpoint(TransactionManager* txn_manager, BufferPoolManager* buffer_pool_manager, SmManager* sm_manager) {
        std::cout << "开始创建静态检查点..." << std::endl;
        
        // 1. 停止接收新事务和正在运行的事务
        txn_manager->pause_transactions();
        std::cout << "已暂停所有事务处理" << std::endl;
        
        try {
            // 2. 将日志缓冲区内容写入磁盘
            flush_log_to_disk();
            std::cout << "已将日志缓冲区刷新到磁盘" << std::endl;
            
            // 获取当前所有活跃事务列表
            std::vector<txn_id_t> active_txns = txn_manager->get_active_transactions();
            std::cout << "当前活跃事务数量: " << active_txns.size() << std::endl;
            
            // 3. 在日志文件中写入检查点记录
            CheckpointLogRecord checkpoint_record(active_txns);
            lsn_t checkpoint_lsn = add_log_to_buffer(&checkpoint_record);
            flush_log_to_disk();  // 确保检查点记录被写入磁盘
            std::cout << "已写入检查点记录，LSN: " << checkpoint_lsn << std::endl;
            
            // 4. 将当前缓冲池中的所有脏页写入磁盘
                // 遍历所有表文件，刷新每个文件的页面
                for (const auto& [table_name, file_handle] : sm_manager->fhs_) {
                        int fd = file_handle->GetFd(); // 获取文件描述符
                        buffer_pool_manager->flush_all_pages(fd);
                        std::cout << "已刷新表 " << table_name << " 的所有页面" << std::endl;
                }
            
            // 5. 将检查点记录的LSN写入重启文件
            std::ofstream restart_file(RESTART_FILE_NAME);
            if (!restart_file.is_open()) {
                throw std::runtime_error("无法创建重启文件: " + RESTART_FILE_NAME);
            }
            restart_file << checkpoint_lsn;
            restart_file.close();
            std::cout << "已将检查点LSN写入重启文件: " << checkpoint_lsn << std::endl;
            
            // 恢复事务处理
            txn_manager->resume_transactions();
            std::cout << "静态检查点创建完成，已恢复事务处理" << std::endl;
        } catch (const std::exception& e) {
            // 出错时恢复事务处理
            txn_manager->resume_transactions();
            std::cerr << "创建检查点失败: " << e.what() << std::endl;
            throw;
        }
    }
    
    /**
     * @description: 获取最后一个检查点的LSN
     * @return {lsn_t} 检查点LSN，如果不存在则返回INVALID_LSN
     */
    lsn_t LogManager::get_last_checkpoint_lsn() {
        // 尝试从重启文件读取检查点LSN
        std::ifstream restart_file(RESTART_FILE_NAME);
        if (!restart_file.is_open()) {
            return INVALID_LSN;  // 文件不存在，返回无效LSN
        }
        
        lsn_t checkpoint_lsn;
        restart_file >> checkpoint_lsn;
        restart_file.close();
        
        return checkpoint_lsn;
    }
    
    /**
     * @description: 从指定位置读取日志记录
     * @param {lsn_t} lsn 日志序列号
     * @return {LogRecord*} 日志记录指针，调用者负责释放内存
     */
    LogRecord* LogManager::read_log_record(lsn_t lsn) {
        // 打开日志文件
        std::ifstream log_file(LOG_FILE_NAME, std::ios::binary);
        if (!log_file.is_open()) {
            throw std::runtime_error("无法打开日志文件");
        }
        
        // 查找日志记录
        char header_buffer[LOG_HEADER_SIZE];
        bool found = false;
        LogRecord* record = nullptr;
        
        while (log_file.read(header_buffer, LOG_HEADER_SIZE)) {
            // 读取日志类型和LSN
            LogType log_type = *reinterpret_cast<const LogType*>(header_buffer + OFFSET_LOG_TYPE);
            lsn_t curr_lsn = *reinterpret_cast<const lsn_t*>(header_buffer + OFFSET_LSN);
            uint32_t log_tot_len = *reinterpret_cast<const uint32_t*>(header_buffer + OFFSET_LOG_TOT_LEN);
            
            // 如果找到指定LSN的日志记录
            if (curr_lsn == lsn) {
                // 重置文件指针到记录开始位置
                log_file.seekg(-LOG_HEADER_SIZE, std::ios::cur);
                
                // 读取完整日志记录
                char* log_data = new char[log_tot_len];
                log_file.read(log_data, log_tot_len);
                
                // 根据日志类型创建对应的日志记录对象
                switch (log_type) {
                    case LogType::begin:
                        record = new BeginLogRecord();
                        break;
                    case LogType::commit:
                        record = new CommitLogRecord();
                        break;
                    case LogType::ABORT:
                        record = new AbortLogRecord();
                        break;
                    case LogType::INSERT:
                        record = new InsertLogRecord();
                        break;
                    case LogType::CHECKPOINT:
                        record = new CheckpointLogRecord();
                        break;
                    default:
                        delete[] log_data;
                        throw std::runtime_error("未知的日志类型");
                }
                
                // 反序列化日志记录
                record->deserialize(log_data);
                delete[] log_data;
                found = true;
                break;
            }
            
            // 跳过当前日志记录的数据部分
            log_file.seekg(log_tot_len - LOG_HEADER_SIZE, std::ios::cur);
        }
        
        log_file.close();
        
        if (!found) {
            throw std::runtime_error("未找到指定LSN的日志记录");
        }
        
        return record;
    }

    /**
 * @description: 从指定LSN开始扫描日志记录
 * @param {lsn_t} start_lsn 开始扫描的LSN，如果为INVALID_LSN则从头开始
 * @return {std::vector<LogRecord*>} 日志记录指针列表，调用者负责释放内存
 */
std::vector<LogRecord*> LogManager::scan_log_from_lsn(lsn_t start_lsn) {
        std::vector<LogRecord*> log_records;
        
        // 打开日志文件
        std::ifstream log_file(LOG_FILE_NAME, std::ios::binary);
        if (!log_file.is_open()) {
            std::cerr << "无法打开日志文件: " << LOG_FILE_NAME << std::endl;
            return log_records;
        }
        
        char header_buffer[LOG_HEADER_SIZE];
        bool found_start = (start_lsn == INVALID_LSN);  // 如果是无效LSN，从头开始扫描
        
        // 逐条读取日志记录
        while (log_file.read(header_buffer, LOG_HEADER_SIZE)) {
            // 解析日志头部，获取类型和LSN
            LogType log_type = *reinterpret_cast<const LogType*>(header_buffer + OFFSET_LOG_TYPE);
            lsn_t curr_lsn = *reinterpret_cast<const lsn_t*>(header_buffer + OFFSET_LSN);
            uint32_t log_tot_len = *reinterpret_cast<const uint32_t*>(header_buffer + OFFSET_LOG_TOT_LEN);
            
            // 如果找到起点或者之后的记录
            if (curr_lsn >= start_lsn || found_start) {
                found_start = true;
                
                // 回退文件指针，以便读取完整记录
                log_file.seekg(-LOG_HEADER_SIZE, std::ios::cur);
                
                // 分配内存并读取完整日志记录
                char* log_data = new char[log_tot_len];
                if (log_file.read(log_data, log_tot_len)) {
                    // 创建对应类型的日志记录对象
                    LogRecord* record = nullptr;
                    
                    switch (log_type) {
                        case LogType::begin:
                            record = new BeginLogRecord();
                            break;
                        case LogType::commit:
                            record = new CommitLogRecord();
                            break;
                        case LogType::ABORT:
                            record = new AbortLogRecord();
                            break;
                        case LogType::INSERT:
                            record = new InsertLogRecord();
                            break;
                        case LogType::UPDATE:
                            record = new UpdateLogRecord();
                            break;
                        case LogType::DELETE:
                            record = new DeleteLogRecord();
                            break;
                        case LogType::CHECKPOINT:
                            record = new CheckpointLogRecord();
                            break;
                        default:
                            // 未知日志类型
                            delete[] log_data;
                            continue;
                    }
                    
                    // 反序列化日志记录
                    record->deserialize(log_data);
                    log_records.push_back(record);
                    
                    delete[] log_data;
                } else {
                    // 读取失败
                    delete[] log_data;
                    break;
                }
            } else {
                // 跳过当前记录
                log_file.seekg(log_tot_len - LOG_HEADER_SIZE, std::ios::cur);
            }
        }
        
        log_file.close();
        
        return log_records;
    }