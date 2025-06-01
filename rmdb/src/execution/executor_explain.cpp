#include "executor_explain.h"
#include "optimizer/planner.h"

// 实现 ExplainExecutor 的方法
void ExplainExecutor::beginTuple() {
    generate_explain_output();
}

void ExplainExecutor::nextTuple() {
    executed_ = true;
}

std::unique_ptr<RmRecord> ExplainExecutor::Next() {
    if (!executed_) {
        // 创建包含EXPLAIN结果的记录
        std::stringstream ss;
        
        // 调用Planner的explain_plan函数生成树状格式的查询计划
        Planner planner(nullptr);
        planner.explain_plan(plan_->plan_, ss);
        
        // 保存结果
        result_ = ss.str();
        
        // 直接输出到终端，方便调试查看
        std::cerr << "\n======= EXPLAIN QUERY PLAN =======\n";
        std::cerr << result_;
        std::cerr << "=================================\n\n";
        
        // 创建记录
        auto rec = std::make_unique<RmRecord>(4096);  // 增大记录大小
        memset(rec->data, 0, 4096);
        memcpy(rec->data, result_.c_str(), result_.length());
        
        executed_ = true;
        return rec;
    }
    return nullptr;
}

bool ExplainExecutor::is_end() const {
    return executed_;
}

const std::vector<ColMeta> &ExplainExecutor::cols() const {
    static std::vector<ColMeta> explain_cols = {
        {.tab_name = "", .name = "EXPLAIN", .type = TYPE_STRING, .len = 4096, .offset = 0}
    };
    return explain_cols;
}

Rid &ExplainExecutor::rid() {
    return _abstract_rid;
}

void ExplainExecutor::generate_explain_output() {
    //beginTuple() 中调用，实际代码已在 Next() 中实现
}

// 实现 ExplainPlan::get_executor 方法
std::unique_ptr<AbstractExecutor> ExplainPlan::get_executor(Context *context) {
    return std::make_unique<ExplainExecutor>(this, context);
}