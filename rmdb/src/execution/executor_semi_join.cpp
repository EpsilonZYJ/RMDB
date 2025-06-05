#include "executor_semi_join.h"
#include <cmath>
#include <cstring>
#include <algorithm>

void SemiJoinExecutor::beginTuple() {
    isend = false;
    matched_left_rids_.clear();
    // 开始处理第一个元组
    left_->beginTuple();
    if (left_->is_end()) {
        isend = true;
        return;
    }
    
    // 查找第一个匹配
    findNextMatch();
}

void SemiJoinExecutor::nextTuple() {
    if (isend) return;
    
    // 当前左表行已处理，移动到下一行
    left_->nextTuple();
    
    if (left_->is_end()) {
        isend = true;
        return;
    }
    
    // 查找下一个匹配
    findNextMatch();
}

// 辅助方法：查找下一个匹配记录
void SemiJoinExecutor::findNextMatch() {
    while (!left_->is_end()) {
        // 获取当前左表记录的RID
        Rid left_rid = left_->rid();
        uint64_t rid_value = (static_cast<uint64_t>(left_rid.page_no) << 32) | left_rid.slot_no;
        
        // 如果该左表行已匹配过，跳过
        if (matched_left_rids_.find(rid_value) != matched_left_rids_.end()) {
            left_->nextTuple();
            continue;
        }
        
        // 保存当前左表记录 - 不要调用Next()，它会移动游标
        current_record_ = std::make_unique<RmRecord>(left_->tuple()->size);
        memcpy(current_record_->data, left_->tuple()->data, left_->tuple()->size);

        // 重置右表，开始扫描
        bool found_match = false;
        right_->beginTuple();
        
        while (!right_->is_end() && !found_match) {
            // 不调用Next()，直接使用tuple()访问数据
            RmRecord* right_tuple = right_->tuple();
            bool match = true;
            
            for (size_t i = 0; i < fed_conds_.size(); i++) {
                // 获取条件列的值
                char* left_value = current_record_->data + left_cols_[i].offset;
                char* right_value = right_tuple->data + right_cols_[i].offset;
                
                bool cond_match = false;
                
                // 根据数据类型和操作符比较值
                switch (fed_conds_[i].op) {
                    case OP_EQ: {
                        if (left_cols_[i].type == TYPE_INT) {
                            cond_match = (*(int*)left_value == *(int*)right_value);
                        } else if (left_cols_[i].type == TYPE_FLOAT) {
                            float diff = *(float*)left_value - *(float*)right_value;
                            cond_match = (std::abs(diff) < 0.000001f);
                        } else if (left_cols_[i].type == TYPE_STRING) {
                            int len_left = *(int*)left_value;
                            int len_right = *(int*)right_value;
                            char* str_left = left_value + sizeof(int);
                            char* str_right = right_value + sizeof(int);
                            cond_match = (len_left == len_right && 
                                         strncmp(str_left, str_right, len_left) == 0);
                        }
                        break;
                    }
                    case OP_NE: {
                        if (left_cols_[i].type == TYPE_INT) {
                            cond_match = (*(int*)left_value != *(int*)right_value);
                        } else if (left_cols_[i].type == TYPE_FLOAT) {
                            float diff = *(float*)left_value - *(float*)right_value;
                            cond_match = (std::abs(diff) >= 0.000001f);
                        } else if (left_cols_[i].type == TYPE_STRING) {
                            int len_left = *(int*)left_value;
                            int len_right = *(int*)right_value;
                            char* str_left = left_value + sizeof(int);
                            char* str_right = right_value + sizeof(int);
                            cond_match = (len_left != len_right || 
                                         strncmp(str_left, str_right, len_left) != 0);
                        }
                        break;
                    }
                    // 实现其他比较操作符...
                    case OP_LT:
                    case OP_GT:
                    case OP_LE:
                    case OP_GE:
                        // 根据需求实现这些操作符
                        break;
                    default:
                        break;
                }
                
                if (!cond_match) {
                    match = false;
                    break;
                }
            }
            
            if (match) {
                // 找到匹配，记录并返回
                matched_left_rids_.insert(rid_value);
                found_match = true;
                return; // SEMI JOIN关键特性：找到一个匹配即可
            }
            
            right_->nextTuple();
        }
        
        // 当前左表行无匹配，继续处理下一行
        if (!found_match) {
            left_->nextTuple();
        }
    }
    
    // 没有更多匹配行
    isend = true;
}

std::unique_ptr<RmRecord> SemiJoinExecutor::Next() {
    // SEMI JOIN只返回左表记录
    return std::make_unique<RmRecord>(*current_record_);
}