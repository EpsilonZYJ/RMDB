#include "executor_explain.h"
#include "optimizer/planner.h"
#include <set>
// 实现 ExplainExecutor 的方法
void ExplainExecutor::beginTuple() {
    generate_explain_output();
}

void ExplainExecutor::nextTuple() {
    executed_ = true;
}

// 辅助函数：按字典序打印项目列表
void print_sorted_items(const std::set<std::string>& items, std::stringstream& ss) {
    bool first = true;
    for (const auto& item : items) {
        if (!first) ss << ",";
        ss << item;
        first = false;
    }
}

void print_sorted_items(const std::vector<std::string>& items, std::stringstream& ss) {
    if (items.empty()) return;
    
    ss << items[0];
    for (size_t i = 1; i < items.size(); ++i) {
        ss << "," << items[i];
    }
}

// 辅助函数：收集JOIN计划中的所有表名
void collect_table_names(std::shared_ptr<JoinPlan> join_plan, std::set<std::string>& table_names) {
    std::function<void(std::shared_ptr<Plan>)> collect = [&](std::shared_ptr<Plan> p) {
        if (!p) return;
        
        if (auto scan = std::dynamic_pointer_cast<ScanPlan>(p)) {
            table_names.insert(scan->tab_name_);
        } else if (auto sub_join = std::dynamic_pointer_cast<JoinPlan>(p)) {
            collect(sub_join->left_);
            collect(sub_join->right_);
        }
    };
    
    collect(join_plan->left_);
    collect(join_plan->right_);
}


void print_plan_tree(std::shared_ptr<Plan> plan, std::stringstream& ss, int indent = 0) {
    std::string tabs(indent, '\t');
    
    if (!plan) return;

    switch (plan->tag) {
        case T_select: {
            // SELECT节点，直接处理子节点
            auto dml_plan = std::dynamic_pointer_cast<DMLPlan>(plan);
            if (dml_plan && dml_plan->subplan_) {
                print_plan_tree(dml_plan->subplan_, ss, indent);
            }
            break;
        }
        case T_SeqScan:
        case T_IndexScan: {
            auto scan_plan = std::dynamic_pointer_cast<ScanPlan>(plan);
            
            // 检查是否有过滤条件 - 如果有，先输出Filter节点
            if (!scan_plan->conds_.empty()) {
                // 收集并排序条件字符串
                std::vector<std::string> conditions;
                for (const auto& cond : scan_plan->conds_) {
                    std::stringstream cond_ss;
                    // 确保使用表名前缀
                    cond_ss << scan_plan->tab_name_ << "." << cond.lhs_col.col_name;
                    
                    switch (cond.op) {
                        case OP_EQ: cond_ss << "="; break;
                        case OP_NE: cond_ss << "<>"; break;
                        case OP_LT: cond_ss << "<"; break;
                        case OP_GT: cond_ss << ">"; break;
                        case OP_LE: cond_ss << "<="; break;
                        case OP_GE: cond_ss << ">="; break;
                        default: cond_ss << "?"; break;
                    }
                    
                    if (cond.is_rhs_val) {
                        // 右侧是常量
                        if (cond.rhs_val.type == TYPE_INT) {
                            cond_ss << cond.rhs_val.int_val;
                        } else if (cond.rhs_val.type == TYPE_FLOAT) {
                            cond_ss << cond.rhs_val.float_val;
                        } else if (cond.rhs_val.type == TYPE_STRING) {
                            cond_ss << "'" << cond.rhs_val.str_val << "'";
                        }
                    } else {
                        // 右侧是列
                        cond_ss << cond.rhs_col.tab_name << "." << cond.rhs_col.col_name;
                    }
                    
                    conditions.push_back(cond_ss.str());
                }
                
                // 按字典序排序条件
                std::sort(conditions.begin(), conditions.end());
                
                // 输出Filter节点
                ss << tabs << "Filter(condition=[";
                for (size_t i = 0; i < conditions.size(); ++i) {
                    if (i > 0) ss << ",";
                    ss << conditions[i];
                }
                ss << "])" << std::endl;
                
                // Filter下嵌套Scan节点
                ss << tabs << "\t" << "Scan(table=" << scan_plan->tab_name_ << ")" << std::endl;
            } else {
                // 无条件的Scan节点
                ss << tabs << "Scan(table=" << scan_plan->tab_name_ << ")" << std::endl;
            }
            break;
        }
        case T_NestLoop: {
            auto join_plan = std::dynamic_pointer_cast<JoinPlan>(plan);
            
            // 收集所有表名并排序
            std::set<std::string> table_names;
            
            // 辅助函数：从计划中收集表名
            std::function<void(std::shared_ptr<Plan>)> collect_tables = [&](std::shared_ptr<Plan> p) {
                if (!p) return;
                
                if (auto scan = std::dynamic_pointer_cast<ScanPlan>(p)) {
                    table_names.insert(scan->tab_name_);
                } else if (auto sub_join = std::dynamic_pointer_cast<JoinPlan>(p)) {
                    collect_tables(sub_join->left_);
                    collect_tables(sub_join->right_);
                }
            };
            
            collect_tables(join_plan->left_);
            collect_tables(join_plan->right_);
            
            // 生成JOIN节点
            ss << tabs << "Join(tables=[";
            bool first = true;
            for (const auto& table : table_names) {
                if (!first) ss << ",";
                ss << table;
                first = false;
            }
            ss << "],condition=[";
            
            // 排序并打印连接条件 
            if (!join_plan->conds_.empty()) {
                std::vector<std::string> join_conditions;
                for (const auto& cond : join_plan->conds_) {
                    if (cond.is_rhs_val) continue; // 跳过非连接条件
                    
                    std::stringstream cond_ss;
                    cond_ss << cond.lhs_col.tab_name << "." << cond.lhs_col.col_name << "=";
                    cond_ss << cond.rhs_col.tab_name << "." << cond.rhs_col.col_name;
                    join_conditions.push_back(cond_ss.str());
                }
                
                std::sort(join_conditions.begin(), join_conditions.end());
                
                first = true;
                for (const auto& condition : join_conditions) {
                    if (!first) ss << ",";
                    ss << condition;
                    first = false;
                }
            }
            ss << "])" << std::endl;
            
            // 递归打印左右子树 - 按题目要求的顺序
            print_plan_tree(join_plan->left_, ss, indent + 1);
            print_plan_tree(join_plan->right_, ss, indent + 1);
            break;
        }
        case T_Projection: {
            auto proj_plan = std::dynamic_pointer_cast<ProjectionPlan>(plan);
            ss << tabs << "Project(columns=[";
            
            if (proj_plan->sel_cols_.empty()) {
                ss << "*";
            } else {
                // 收集并排序列名
                std::vector<std::string> column_names;
                for (const auto& col : proj_plan->sel_cols_) {
                    column_names.push_back(col.tab_name + "." + col.col_name);
                }
                std::sort(column_names.begin(), column_names.end());
                
                bool first = true;
                for (const auto& col_name : column_names) {
                    if (!first) ss << ",";
                    ss << col_name;
                    first = false;
                }
            }
            ss << "])" << std::endl;
            
            // 打印子计划
            if (proj_plan->subplan_) {
                print_plan_tree(proj_plan->subplan_, ss, indent + 1);
            }
            break;
        }
        case T_Sort: {
            // 这里应简化为适当节点
            auto sort_plan = std::dynamic_pointer_cast<SortPlan>(plan);
            if (sort_plan->subplan_) {
                print_plan_tree(sort_plan->subplan_, ss, indent);
            }
            break;
        }
        case T_Explain: {
            auto explain_plan = std::dynamic_pointer_cast<ExplainPlan>(plan);
            if (explain_plan && explain_plan->plan_) {
                print_plan_tree(explain_plan->plan_, ss, indent);
            }
            break;
        }
        default:
            ss << tabs << "Unknown plan type: " << plan->tag << std::endl;
            break;
    }
}

std::unique_ptr<RmRecord> ExplainExecutor::Next() {
    if (!executed_) {
        // 创建包含EXPLAIN结果的记录
        std::stringstream ss;
        
        print_plan_tree(plan_->plan_, ss);


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

