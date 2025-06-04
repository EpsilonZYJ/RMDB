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

class ProjectionExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> prev_;        // 投影节点的儿子节点  // 只会有一个儿子
    std::vector<ColMeta> cols_;                     // 需要投影的字段
    size_t len_;                                    // 字段总长度
    std::vector<size_t> sel_idxs_;                  // 需要投影的字段在原始输入列中的位置
    
    bool agg_ = false;

   public:
    ProjectionExecutor(std::unique_ptr<AbstractExecutor> prev, const std::vector<TabCol> &sel_cols) {
        prev_ = std::move(prev);
        size_t curr_offset = 0;

        if(prev_->getType() == "AggregateExecutor") {
            agg_ = true;
            for (auto &col_meta: prev_->cols()) {
                cols_.emplace_back(col_meta);
                cols_.back().offset = curr_offset;
                curr_offset += col_meta.len;
            }
            return;
        }

        auto &prev_cols = prev_->cols();
        for (auto &sel_col : sel_cols) {
            auto pos = get_col(prev_cols, sel_col);
            sel_idxs_.push_back(pos - prev_cols.begin());
            auto col = *pos;
            col.offset = curr_offset;
            curr_offset += col.len;
            cols_.push_back(col);
        }
        len_ = curr_offset;
    }

    void beginTuple() override { prev_->beginTuple(); }

    void nextTuple() override { prev_->nextTuple(); }

    std::unique_ptr<RmRecord> Next() override {
        if(agg_) {
            // 由于AggregateExecutor的beginTuple()可能会更新cols_，因此需要重新获取cols_
            // TODO 这只是补救的方法，可能会有其他更好的方法
            size_t curr_offset = 0;
            cols_.clear();
            for (auto &col_meta: prev_->cols()) {
                cols_.emplace_back(col_meta);
                cols_.back().offset = curr_offset;
                curr_offset += col_meta.len;
            }
            return std::move(prev_->Next());
        }

        auto rec = prev_->Next();
        auto &prev_cols = prev_->cols();

        std::unique_ptr<RmRecord> new_rec = std::make_unique<RmRecord>(len_);
        int cur_offset = 0;

        for (size_t i = 0; i < cols_.size(); i++){
            size_t col_offset = prev_cols[sel_idxs_[i]].offset;
            memcpy(new_rec->data + cur_offset, rec->data + col_offset, cols_[i].len);
            cur_offset += cols_[i].len;
        }

        return new_rec;
    }

    bool is_end() const override { return prev_->is_end(); }

    const std::vector<ColMeta> &cols() const override { return cols_; }

    size_t tupleLen() const override { return len_; }

    Rid &rid() override { return _abstract_rid; }
};