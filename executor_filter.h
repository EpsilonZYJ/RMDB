#pragma once

#include "execution_defs.h"
#include "execution/executor_abstract.h"
#include "common/context.h"
#include "optimizer/plan.h"
#include "check_condition.h"
class FilterExecutor : public AbstractExecutor {
private:
    std::unique_ptr<AbstractExecutor> subplan_executor_;
    std::vector<Condition> conds_;
    bool executed_;

public:
    FilterExecutor(std::shared_ptr<FilterPlan> plan, std::unique_ptr<AbstractExecutor> child_executor) 
    : subplan_executor_(std::move(child_executor)), conds_(plan->conds_), executed_(false) {}

    void beginTuple() override;
    void nextTuple() override;
    std::unique_ptr<RmRecord> Next() override;
    bool is_end() const override;
    const std::vector<ColMeta> &cols() const override;
    Rid &rid() override;
};

//创建执行器的辅助函数
// std::unique_ptr<AbstractExecutor> create_executor_for_plan(std::shared_ptr<Plan> plan, Context* context);