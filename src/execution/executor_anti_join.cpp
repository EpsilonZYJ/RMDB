#include "executor_anti_join.h"
#include <cmath>
#include <cstring>
void AntiJoinExecutor::beginTuple() {
    std::cout << "DEBUG: AntiJoinExecutor::beginTuple() 开始" << std::endl;
    isend = false;
    processed_left_rids_.clear();
    current_matching_record_ = nullptr; 
    // 遍历左表
    left_->beginTuple();
    if (left_->is_end()) {
        isend = true;
        return;
    }
    // 找到第一个匹配的记录
    findNextMatch();
}

void AntiJoinExecutor::nextTuple() {
    if (isend) return;
    std::cout << "DEBUG: AntiJoinExecutor::nextTuple() 寻找下一个匹配" << std::endl;
    // 移动到下一个左表记录
    left_->nextTuple();
    
    if (left_->is_end()) {
        isend = true;
        current_matching_record_ = nullptr;
        return;
    }
    // 寻找下一个匹配记录
    findNextMatch();
}

void AntiJoinExecutor::findNextMatch() {
    while (!left_->is_end()) {
        // 获取当前左表记录的rid
        Rid left_rid = left_->rid();
        uint64_t rid_value = (static_cast<uint64_t>(left_rid.page_no) << 32) | left_rid.slot_no;
        
        // 跳过已处理的记录
        if (processed_left_rids_.find(rid_value) != processed_left_rids_.end()) {
            left_->nextTuple();
            continue;
        }
        // 获取并缓存当前左表记录
        std::unique_ptr<RmRecord> left_record = left_->Next();
        // 左表已经移动，需要再次初始化
        left_->beginTuple();
        // 重新定位到相同位置
        while (!left_->is_end()) {
            if (left_->rid().page_no == left_rid.page_no && 
                left_->rid().slot_no == left_rid.slot_no) {
                break;
            }
            left_->nextTuple();
        }
        
        // 检查该左表记录是否与任何右表记录匹配
        bool found_match = false;
        right_->beginTuple();
        
        while (!right_->is_end()) {
            std::unique_ptr<RmRecord> right_record = right_->Next();
            bool match = true;
            // 检查所有连接条件
            for (size_t i = 0; i < fed_conds_.size(); i++) {
                char* left_value = left_record->data + left_cols_[i].offset;
                char* right_value = right_record->data + right_cols_[i].offset;
                bool cond_match = compareValues(left_value, right_value, left_cols_[i].type, fed_conds_[i].op);
                if (!cond_match) {
                    match = false;
                    break;
                }
            }
            if(match==true){
                found_match=true;
                break;
            }
            right_->nextTuple();
        }
        if (found_match) {
            std::cout << "DEBUG: 左表记录 " << rid_value << " 无匹配" << std::endl;
            left_->nextTuple();
        }
            
        // 未找到不匹配，继续下一个左表记录
        if (!found_match) {
            // 找到不匹配
            found_match = true;
            current_matching_record_ = std::move(left_record);
            processed_left_rids_.insert(rid_value);
            std::cout << "DEBUG: 找到不匹配的左表记录 " << rid_value << std::endl;
            return;
        }
    }
    
    // 没有更多匹配记录
    isend = true;
    current_matching_record_ = nullptr;
    std::cout << "DEBUG: 反半连接遍历完成，无更多匹配" << std::endl;
}