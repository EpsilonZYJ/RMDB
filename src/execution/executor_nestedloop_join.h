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
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"
#include "check_condition.h"

class NestedLoopJoinExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> left_;    // 左儿子节点（需要join的表）
    std::unique_ptr<AbstractExecutor> right_;   // 右儿子节点（需要join的表）
    size_t len_;                                // join后获得的每条记录的长度
    std::vector<ColMeta> cols_;                 // join后获得的记录的字段
    std::vector<ColMeta> left_cols_;            // fed_conds_中左表的字段，size应该与fed_conds_.size()相同
    std::vector<ColMeta> right_cols_;           // fed_conds_中右表的字段，size应该与fed_conds_.size()相同

    std::vector<Condition> fed_conds_;          // join条件
    bool isend;                                 // 是否已经遍历完所有记录   

   public:
    NestedLoopJoinExecutor(std::unique_ptr<AbstractExecutor> left, std::unique_ptr<AbstractExecutor> right, 
                            std::vector<Condition> conds) {
        left_ = std::move(left);
        right_ = std::move(right);
        len_ = left_->tupleLen() + right_->tupleLen();
        cols_ = left_->cols();
        auto right_cols = right_->cols();
        for (auto &col : right_cols) {
            col.offset += left_->tupleLen();
        }

        cols_.insert(cols_.end(), right_cols.begin(), right_cols.end());
        isend = false;
        fed_conds_ = std::move(conds);
        for(auto &cond: fed_conds_) {
            left_cols_.push_back(*get_col(cols_, cond.lhs_col));
            right_cols_.push_back(*get_col(cols_, cond.rhs_col));
        }
    }

    // void beginTuple() override {
    //     isend = false;

    //     for(left_->beginTuple(); !(left_->is_end()); left_->nextTuple()) { // 外层循环
    //         for(right_->beginTuple(); !(right_->is_end()); right_->nextTuple()) { // 内层循环
    //             auto rec_left = left_->Next();
    //             auto rec_right = right_->Next();

    //             std::unique_ptr<RmRecord> new_rec = std::make_unique<RmRecord>(len_);
    //             memcpy(new_rec->data, rec_left->data, left_->tupleLen());
    //             memcpy(new_rec->data + left_->tupleLen(), rec_right->data, right_->tupleLen());
    //             if(check_condition(
    //                 *new_rec, 
    //                 left_cols_,
    //                 right_cols_, 
    //                 fed_conds_)
    //             ) return; // TODO:如果右值不是value，这里是不是也正确？
    //         }
    //     }

    //     isend = true;       
    // }

    // void nextTuple() override {
    //     right_->nextTuple(); // 先将右表的指针移动到下一条记录

    //     for(; !(left_->is_end()); left_->nextTuple()) { // 外层循环
    //         for(; !(right_->is_end()); right_->nextTuple()) { // 内层循环
    //             auto rec_left = left_->Next();
    //             auto rec_right = right_->Next();

    //             std::unique_ptr<RmRecord> new_rec = std::make_unique<RmRecord>(len_);
    //             memcpy(new_rec->data, rec_left->data, left_->tupleLen());
    //             memcpy(new_rec->data + left_->tupleLen(), rec_right->data, right_->tupleLen());
    //             if(check_condition(
    //                 *new_rec, 
    //                 left_cols_,
    //                 right_cols_, 
    //                 fed_conds_)
    //             ) return; // TODO:如果右值不是value，这里是不是也正确？
    //         }
    //         right_->beginTuple(); // 内层循环结束后，重新开始右表的遍历（这句话不能用于内循环的初始化）
    //     }

    //     isend = true;  
    // }
    void beginTuple() override {
        isend = false;
        // 初始化左表
        left_->beginTuple();
        if (left_->is_end()) {
            isend = true;
            return;
        }
        // 初始化右表
        right_->beginTuple();
        // 找到第一个满足条件的结果
        while (true) {
            if (right_->is_end()) {
                // 右表遍历完，移到左表下一行
                left_->nextTuple();
                if (left_->is_end()) {
                    isend = true;
                    return;
                }
                right_->beginTuple(); // 重新开始右表
            }
            
            // 检查当前组合是否满足条件
            auto rec_left = left_->Next();
            auto rec_right = right_->Next();
            
            std::unique_ptr<RmRecord> new_rec = std::make_unique<RmRecord>(len_);
            memcpy(new_rec->data, rec_left->data, left_->tupleLen());
            memcpy(new_rec->data + left_->tupleLen(), rec_right->data, right_->tupleLen());
            
            if (check_condition(*new_rec, left_cols_, right_cols_, fed_conds_)) {
                return; // 找到满足条件的结果，不返回记录但保持当前状态
            }
            // 当前组合不满足条件，移到右表下一行
            right_->nextTuple();
        }
    }
    
    void nextTuple() override {
        // 移到下一个组合
        right_->nextTuple();
        // 查找下一个满足条件的组合
        while (true) {
            if (right_->is_end()) {
                // 右表遍历完，移到左表下一行
                left_->nextTuple();
                if (left_->is_end()) {
                    isend = true;
                    return;
                }
                right_->beginTuple(); // 重新开始右表
            }
            // 检查当前组合
            auto rec_left = left_->Next();
            auto rec_right = right_->Next();
            
            std::unique_ptr<RmRecord> new_rec = std::make_unique<RmRecord>(len_);
            memcpy(new_rec->data, rec_left->data, left_->tupleLen());
            memcpy(new_rec->data + left_->tupleLen(), rec_right->data, right_->tupleLen());
            
            if (check_condition(*new_rec, left_cols_, right_cols_, fed_conds_)) {
                return; // 找到满足条件的结果
            }
            // 当前组合不满足条件，移到右表下一行
            right_->nextTuple();
        }
    }
    std::unique_ptr<RmRecord> Next() override {
        auto rec_left = left_->Next();
        auto rec_right = right_->Next();

        std::unique_ptr<RmRecord> new_rec = std::make_unique<RmRecord>(len_);
        memcpy(new_rec->data, rec_left->data, left_->tupleLen());
        memcpy(new_rec->data + left_->tupleLen(), rec_right->data, right_->tupleLen());
        
        return new_rec;
    }

    bool is_end() const override { return isend; }

    const std::vector<ColMeta> &cols() const override { return cols_; }

    size_t tupleLen() const override { return len_; }

    Rid &rid() override { return _abstract_rid; }
};