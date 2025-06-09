#pragma once

#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"

class LimitExecutor : public AbstractExecutor {
private:
    std::unique_ptr<AbstractExecutor> prev_;  // 前置执行器
    int limit_num_;  // 限制的元组数量
    int current_count_;  // 当前已返回的元组数量
public:
    LimitExecutor(std::unique_ptr<AbstractExecutor> prev, int limit_num)
        : prev_(std::move(prev)), limit_num_(limit_num), current_count_(0) {}

    void beginTuple() override {
        current_count_ = 1; // 重置计数器
        prev_->beginTuple(); // 调用前置执行器的beginTuple
    }

    void nextTuple() override {
        current_count_++; // 增加计数
        if (is_end()) {
            return; // 达到限制，不返回更多元组
        }

        prev_->nextTuple(); // 调用前置执行器的nextTuple
    }

    std::unique_ptr<RmRecord> Next() override {
        return prev_->Next(); // 返回前置执行器的下一个记录
    }

    Rid &rid() override { throw InternalError("AggregateExecutor does not have valid RID"); }

    bool is_end() const {
        return prev_->is_end() || current_count_ > limit_num_;
    }

    const std::vector<ColMeta> &cols() const override { return prev_->cols(); }

    size_t tupleLen() const override { return prev_->tupleLen(); }

    std::string getType() { return "AggregateExecutor"; }
};