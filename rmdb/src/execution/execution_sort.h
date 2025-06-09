#pragma once

#include <queue>
#include <memory>
#include <fstream>
#include <algorithm>
#include <functional>
#include "executor_abstract.h"

// 可通过宏开关选择内存排序或外部排序
// #define EXTERNAL_SORT
#define MAX_CHUNK_SIZE 5

class SortExecutor : public AbstractExecutor {
private:
// ===== 基本属性 =====
    std::unique_ptr<AbstractExecutor> prev_;  // 上游执行器
    ColMeta sort_col_;                        // 排序列
    size_t tuple_len_;                        // 元组长度
    bool is_desc_;                            // 是否降序
    bool is_end_{false};                      // 是否结束
    
// ===== 记录管理 =====
    std::unique_ptr<RmRecord> current_tuple_;          // 当前返回的元组
    std::deque<std::unique_ptr<RmRecord>> records_;    // 内存中的记录
    
// ===== 外部排序相关 =====
    std::string output_filename_;                      // 最终排序结果文件
    std::deque<std::string> temp_files_;               // 临时文件列表
    size_t instance_id_;                               // 实例ID（避免文件名冲突）
    
    // 生成唯一实例ID
    static std::size_t generateID() {
        static size_t current_id = 0;
        return ++current_id;
    }
    
// ===== 比较函数 =====
    int compare(const char* lhs, const char* rhs, int len, ColType type) const {
        switch (type) {
            case TYPE_INT: {
                int lhs_val = *reinterpret_cast<const int*>(lhs);
                int rhs_val = *reinterpret_cast<const int*>(rhs);
                return (lhs_val < rhs_val) ? -1 : (lhs_val > rhs_val) ? 1 : 0;
            }
            case TYPE_FLOAT: {
                float lhs_val = *reinterpret_cast<const float*>(lhs);
                float rhs_val = *reinterpret_cast<const float*>(rhs);
                return (lhs_val < rhs_val) ? -1 : (lhs_val > rhs_val) ? 1 : 0;
            }
            case TYPE_STRING: {
                return std::strncmp(lhs, rhs, len);
            }
            default:
                throw InternalError("Unsupported column type for sorting.");
        }
    }

    // 记录比较函数（用于排序）
    auto getRecordComparator() const {
        return [this](const std::unique_ptr<RmRecord>& lhs, const std::unique_ptr<RmRecord>& rhs) {
            const char* lhs_field = lhs->data + sort_col_.offset;
            const char* rhs_field = rhs->data + sort_col_.offset;
            int cmp_result = compare(lhs_field, rhs_field, sort_col_.len, sort_col_.type); // TODO 排序升序降序好像有点问题
            return is_desc_ ? cmp_result > 0 : cmp_result < 0;
        };
    }
    
    // 最小堆比较函数（用于归并）
    auto getHeapComparator() const {
        return [this](const std::pair<std::unique_ptr<RmRecord>, std::ifstream*>& lhs,
                      const std::pair<std::unique_ptr<RmRecord>, std::ifstream*>& rhs) {
            const char* lhs_field = lhs.first->data + sort_col_.offset;
            const char* rhs_field = rhs.first->data + sort_col_.offset;
            int cmp_result = compare(lhs_field, rhs_field, sort_col_.len, sort_col_.type);
            // 最小堆需要反向比较结果
            return is_desc_ ? cmp_result < 0 : cmp_result > 0;
        };
    }

// ===== 内存排序 =====
    void performInMemorySort() {
        // 读取所有记录到内存并排序
        for (prev_->beginTuple(); !prev_->is_end(); prev_->nextTuple()) {
            records_.emplace_back(prev_->Next());
        }
        
        if (records_.empty()) {
            is_end_ = true;
            return;
        }
        
        // 排序
        std::sort(records_.begin(), records_.end(), getRecordComparator());
    }
    
// ===== 外部排序 =====
    // 排序并保存一个块
    void sortAndSaveChunk() {
        if (records_.empty()) return;
        
        // 对块内记录排序
        std::sort(records_.begin(), records_.end(), getRecordComparator());
        
        // 创建临时文件
        std::string temp_filename = "sorted_chunk_" + std::to_string(instance_id_) + "_" + 
                                   std::to_string(temp_files_.size()) + ".tmp";
        std::ofstream temp_file(temp_filename, std::ios::binary); // 第二个参数表示用二进制模式打开文件
        
        if (!temp_file) {
            throw InternalError("Failed to create temporary file: " + temp_filename);
        }
        
        // 写入记录
        for (const auto& record : records_) {
            temp_file.write(record->data, record->size);
        }
        
        temp_file.close();
        temp_files_.push_back(temp_filename);
        records_.clear();
    }
    
    // 外部多路归并
    void performExternalSort() {
        // 读取所有记录并分块排序
        for (prev_->beginTuple(); !prev_->is_end(); prev_->nextTuple()) {
            records_.emplace_back(prev_->Next());
            
            // 达到块大小限制时排序并写入磁盘
            if (records_.size() >= MAX_CHUNK_SIZE) {
                sortAndSaveChunk();
            }
        }
        
        // 处理剩余记录
        if (!records_.empty()) {
            sortAndSaveChunk();
        }
        
        // 空表处理
        if (temp_files_.empty()) {
            is_end_ = true;
            return;
        }
        
        // 归并所有块
        mergeSortedChunks();
    }
    
    // 多路归并排序后的块
    void mergeSortedChunks() {
        const size_t MERGE_BATCH_SIZE = 2; // 二路归并
        size_t merge_level = 1;
        
        // 当还有多个临时文件时，继续归并
        while (temp_files_.size() > 1) {
            size_t total_files = temp_files_.size();
            size_t processed_files = 0;
            std::deque<std::string> next_level_files;
            
            // 批量处理文件
            while (processed_files < total_files) {
                // 打开当前批次的文件
                std::vector<std::ifstream> file_streams;
                for (size_t i = 0; i < MERGE_BATCH_SIZE && processed_files < total_files; ++i, ++processed_files) {
                    const auto& filename = temp_files_[processed_files];
                    file_streams.emplace_back(filename, std::ios::binary);
                    if (!file_streams.back()) {
                        throw InternalError("Failed to open temporary file: " + filename);
                    }
                }
                
                // 准备归并堆
                using HeapItem = std::pair<std::unique_ptr<RmRecord>, std::ifstream*>;
                std::priority_queue<HeapItem, std::vector<HeapItem>, decltype(getHeapComparator())> 
                    merge_heap(getHeapComparator());
                
                // 初始化堆
                for (auto& file_stream : file_streams) {
                    auto record = std::make_unique<RmRecord>(tuple_len_);
                    file_stream.read(record->data, record->size);
                    if (file_stream.gcount() > 0) {
                        merge_heap.emplace(std::move(record), &file_stream); 
                    }
                }
                
                // 创建输出文件
                std::string merged_filename = "sorted_" + std::to_string(instance_id_) + 
                                             "_level" + std::to_string(merge_level) + "_" + 
                                             std::to_string(next_level_files.size()) + ".tmp"; // e.g. sorted_1_level1_0.tmp
                std::ofstream merged_file(merged_filename, std::ios::binary);
                next_level_files.push_back(merged_filename);
                
                // 执行归并
                while (!merge_heap.empty()) {
                    auto top_item = std::move(const_cast<HeapItem&>(merge_heap.top()));
                    merge_heap.pop();
                    auto& record = top_item.first;
                    auto file_stream = top_item.second;
                    
                    // 写入当前最小/最大记录
                    merged_file.write(record->data, record->size);
                    
                    // 读取下一条记录
                    auto next_record = std::make_unique<RmRecord>(tuple_len_);
                    file_stream->read(next_record->data, next_record->size);
                    if (file_stream->gcount() > 0) {
                        merge_heap.emplace(std::move(next_record), file_stream);
                    }
                }
                
                merged_file.close();
            }
            
            // 删除当前级别的临时文件
            for (const auto& filename : temp_files_) {
                remove(filename.c_str());
            }
            
            // 准备下一级归并
            temp_files_ = std::move(next_level_files);
            ++merge_level;
        }
        
        // 最终只剩一个文件，即为排序结果
        if (temp_files_.size() == 1) {
            output_filename_ = std::move(temp_files_[0]);
            temp_files_.clear();
        }
    }
    
    // 清理临时文件
    void cleanupTempFiles() {
        for (const auto& filename : temp_files_) {
            remove(filename.c_str());
        }
        temp_files_.clear();
    }

public:
    // 构造函数
    SortExecutor(std::unique_ptr<AbstractExecutor> prev, const TabCol& sort_col, bool is_desc) 
        : prev_(std::move(prev)), 
          is_desc_(is_desc),
          tuple_len_(prev_->tupleLen()),
          instance_id_(generateID()) {
        
        if(prev_->getType() == "AggregateExecutor") {
            prev_->beginTuple(); // 确保AggregateExecutor开始执行,因为它有可能修改cols_
        }

        // 获取排序列的元数据
        sort_col_ = *get_col(prev_->cols(), sort_col);
    }
    
    // 析构函数，确保资源清理
    ~SortExecutor() {
        cleanupTempFiles();
    }

    // 开始执行排序
    void beginTuple() override {
        records_.clear();
        cleanupTempFiles();
        is_end_ = false;
        
        
#ifdef EXTERNAL_SORT
        // 外部排序
        performExternalSort();
#else
        // 内存排序
        performInMemorySort();
        
        // 准备第一条记录
        if (!records_.empty() && !is_end_) {
            current_tuple_ = std::move(records_.front());
            records_.pop_front();
        } else {
            is_end_ = true;
        }
#endif
    }

    // 移动到下一条记录
    void nextTuple() override {
        if (is_end_) return;
        
        // 如果内存缓冲区还有记录，直接使用
        if (!records_.empty()) {
            current_tuple_ = std::move(records_.front());
            records_.pop_front();
            return;
        }
        
#ifdef EXTERNAL_SORT
        // 外部排序模式：从文件读取下一批记录
        fetchNextBatchFromFile();
        if (!records_.empty()) {
            current_tuple_ = std::move(records_.front());
            records_.pop_front();
        } else {
            is_end_ = true;
            current_tuple_ = nullptr;
        }
#else
        // 内存排序模式：所有记录已处理完
        is_end_ = true;
        current_tuple_ = nullptr;
#endif
    }

    // 获取当前记录
    std::unique_ptr<RmRecord> Next() override {
        return std::move(current_tuple_);
    }

    // 其他必要实现
    Rid& rid() override { return _abstract_rid; }
    bool is_end() const override { return is_end_; }
    const std::vector<ColMeta>& cols() const override { return prev_->cols(); }
    size_t tupleLen() const override { return tuple_len_; }
    std::string getType() override { return "SortExecutor"; }
};