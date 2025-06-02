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

    // 处理EXPLAIN和SELECT节点
    if (plan->tag == T_Explain || plan->tag == T_select) {
        if (plan->tag == T_Explain) {
            auto explain_plan = std::dynamic_pointer_cast<ExplainPlan>(plan);
            if (explain_plan && explain_plan->plan_) {
                print_plan_tree(explain_plan->plan_, ss, indent);
            }
        } else {
            auto dml_plan = std::dynamic_pointer_cast<DMLPlan>(plan);
            if (dml_plan && dml_plan->subplan_) {
                print_plan_tree(dml_plan->subplan_, ss, indent);
            }
        }
        return;
    }

     // 处理投影节点
     if (plan->tag == T_Projection) {
        auto proj_plan = std::dynamic_pointer_cast<ProjectionPlan>(plan);
        ss << tabs << "Project(columns=[";
        
        // 使用is_select_star_标志
        if (proj_plan->is_select_star_) {
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
        return;
    }
    
    // 处理连接节点
    if (plan->tag == T_NestLoop) {
        auto join_plan = std::dynamic_pointer_cast<JoinPlan>(plan);
        
        // 收集所有表名并排序
        std::set<std::string> table_names;
        collect_table_names(join_plan, table_names);
        
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
        std::vector<std::string> join_conditions;
        for (const auto& cond : join_plan->conds_) {
            if (cond.is_rhs_val) continue;
            
            std::stringstream cond_ss;
            cond_ss << cond.lhs_col.tab_name << "." << cond.lhs_col.col_name << "=";
            cond_ss << cond.rhs_col.tab_name << "." << cond.rhs_col.col_name;
            join_conditions.push_back(cond_ss.str());
        }
        
        std::sort(join_conditions.begin(), join_conditions.end());
        first = true;
        for (const auto& cond : join_conditions) {
            if (!first) ss << ",";
            ss << cond;
            first = false;
        }
        ss << "])" << std::endl;
        
        // 关键修改：确定子树对应的表名
    std::string left_table = "";
    std::string right_table = "";
    
    // 检查左右子树是否为扫描节点或其上有投影节点
    auto get_table_name = [](std::shared_ptr<Plan> p) -> std::string {
        if (auto scan = std::dynamic_pointer_cast<ScanPlan>(p)) {
            return scan->tab_name_;
        } else if (auto proj = std::dynamic_pointer_cast<ProjectionPlan>(p)) {
            if (auto scan = std::dynamic_pointer_cast<ScanPlan>(proj->subplan_)) {
                return scan->tab_name_;
            }
        }
        return "";
    };
    
    left_table = get_table_name(join_plan->left_);
    right_table = get_table_name(join_plan->right_);
    
    // 根据表名字典序决定输出顺序
    if (!left_table.empty() && !right_table.empty()) {
        if (left_table > right_table) {
            // 如果左表名大于右表名，先处理右子树，再处理左子树
            
            // 处理右子树
            if (auto right_scan = std::dynamic_pointer_cast<ScanPlan>(join_plan->right_)) {
                if (!right_scan->conds_.empty()) {
                    // 输出Filter节点
                    ss << tabs << "\tFilter(condition=[";
                    std::vector<std::string> filter_conds;
                    for (const auto& cond : right_scan->conds_) {
                        std::stringstream cond_ss;
                        cond_ss << right_scan->tab_name_ << "." << cond.lhs_col.col_name;
                        
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
                            if (cond.rhs_val.type == TYPE_INT) {
                                cond_ss << cond.rhs_val.int_val;
                            } else if (cond.rhs_val.type == TYPE_FLOAT) {
                                cond_ss << cond.rhs_val.float_val;
                            } else {
                                cond_ss << "'" << cond.rhs_val.str_val << "'";
                            }
                        }
                        filter_conds.push_back(cond_ss.str());
                    }
                    
                    std::sort(filter_conds.begin(), filter_conds.end());
                    first = true;
                    for (const auto& cond : filter_conds) {
                        if (!first) ss << ",";
                        ss << cond;
                        first = false;
                    }
                    ss << "])" << std::endl;
                    
                    // 输出Scan节点
                    ss << tabs << "\t\tScan(table=" << right_scan->tab_name_ << ")" << std::endl;
                } else {
                    // 没有过滤条件，直接输出Scan
                    ss << tabs << "\tScan(table=" << right_scan->tab_name_ << ")" << std::endl;
                }
            } else {
                // 不是简单的扫描节点，递归处理
                print_plan_tree(join_plan->right_, ss, indent + 1);
            }
            
            // 处理左子树
            if (auto left_scan = std::dynamic_pointer_cast<ScanPlan>(join_plan->left_)) {
                if (!left_scan->conds_.empty()) {
                    // 输出Filter节点
                    ss << tabs << "\tFilter(condition=[";
                    std::vector<std::string> filter_conds;
                    for (const auto& cond : left_scan->conds_) {
                        std::stringstream cond_ss;
                        cond_ss << left_scan->tab_name_ << "." << cond.lhs_col.col_name;
                        
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
                            if (cond.rhs_val.type == TYPE_INT) {
                                cond_ss << cond.rhs_val.int_val;
                            } else if (cond.rhs_val.type == TYPE_FLOAT) {
                                cond_ss << cond.rhs_val.float_val;
                            } else {
                                cond_ss << "'" << cond.rhs_val.str_val << "'";
                            }
                        }
                        filter_conds.push_back(cond_ss.str());
                    }
                    
                    std::sort(filter_conds.begin(), filter_conds.end());
                    first = true;
                    for (const auto& cond : filter_conds) {
                        if (!first) ss << ",";
                        ss << cond;
                        first = false;
                    }
                    ss << "])" << std::endl;
                    
                    // 输出Scan节点
                    ss << tabs << "\t\tScan(table=" << left_scan->tab_name_ << ")" << std::endl;
                } else {
                    // 没有过滤条件，直接输出Scan
                    ss << tabs << "\tScan(table=" << left_scan->tab_name_ << ")" << std::endl;
                }
            } else {
                // 不是简单的扫描节点，递归处理
                print_plan_tree(join_plan->left_, ss, indent + 1);
            }
        } else {
            // 默认顺序：先处理左子树，再处理右子树
            
            // 处理左子树
            if (auto left_scan = std::dynamic_pointer_cast<ScanPlan>(join_plan->left_)) {
                if (!left_scan->conds_.empty()) {
                    // 输出Filter节点
                    ss << tabs << "\tFilter(condition=[";
                    std::vector<std::string> filter_conds;
                    for (const auto& cond : left_scan->conds_) {
                        std::stringstream cond_ss;
                        cond_ss << left_scan->tab_name_ << "." << cond.lhs_col.col_name;
                        
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
                            if (cond.rhs_val.type == TYPE_INT) {
                                cond_ss << cond.rhs_val.int_val;
                            } else if (cond.rhs_val.type == TYPE_FLOAT) {
                                cond_ss << cond.rhs_val.float_val;
                            } else {
                                cond_ss << "'" << cond.rhs_val.str_val << "'";
                            }
                        }
                        filter_conds.push_back(cond_ss.str());
                    }
                    
                    std::sort(filter_conds.begin(), filter_conds.end());
                    first = true;
                    for (const auto& cond : filter_conds) {
                        if (!first) ss << ",";
                        ss << cond;
                        first = false;
                    }
                    ss << "])" << std::endl;
                    
                    // 输出Scan节点
                    ss << tabs << "\t\tScan(table=" << left_scan->tab_name_ << ")" << std::endl;
                } else {
                    // 没有过滤条件，直接输出Scan
                    ss << tabs << "\tScan(table=" << left_scan->tab_name_ << ")" << std::endl;
                }
            } else {
                // 不是简单的扫描节点，递归处理
                print_plan_tree(join_plan->left_, ss, indent + 1);
            }
            
            // 处理右子树
            if (auto right_scan = std::dynamic_pointer_cast<ScanPlan>(join_plan->right_)) {
                if (!right_scan->conds_.empty()) {
                    // 输出Filter节点
                    ss << tabs << "\tFilter(condition=[";
                    std::vector<std::string> filter_conds;
                    for (const auto& cond : right_scan->conds_) {
                        std::stringstream cond_ss;
                        cond_ss << right_scan->tab_name_ << "." << cond.lhs_col.col_name;
                        
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
                            if (cond.rhs_val.type == TYPE_INT) {
                                cond_ss << cond.rhs_val.int_val;
                            } else if (cond.rhs_val.type == TYPE_FLOAT) {
                                cond_ss << cond.rhs_val.float_val;
                            } else {
                                cond_ss << "'" << cond.rhs_val.str_val << "'";
                            }
                        }
                        filter_conds.push_back(cond_ss.str());
                    }
                    
                    std::sort(filter_conds.begin(), filter_conds.end());
                    first = true;
                    for (const auto& cond : filter_conds) {
                        if (!first) ss << ",";
                        ss << cond;
                        first = false;
                    }
                    ss << "])" << std::endl;
                    
                    // 输出Scan节点
                    ss << tabs << "\t\tScan(table=" << right_scan->tab_name_ << ")" << std::endl;
                } else {
                    // 没有过滤条件，直接输出Scan
                    ss << tabs << "\tScan(table=" << right_scan->tab_name_ << ")" << std::endl;
                }
            } else {
                // 不是简单的扫描节点，递归处理
                print_plan_tree(join_plan->right_, ss, indent + 1);
            }
        }
    } else {
        // 无法确定表名，使用默认处理逻辑
        print_plan_tree(join_plan->left_, ss, indent + 1);
        print_plan_tree(join_plan->right_, ss, indent + 1);
    }
    
    return;
}
    
    // 处理扫描和过滤节点
    if (plan->tag == T_SeqScan || plan->tag == T_IndexScan) {
        auto scan_plan = std::dynamic_pointer_cast<ScanPlan>(plan);
        
        // 检查是否有过滤条件
        if (!scan_plan->conds_.empty()) {
            // 输出Filter节点
            ss << tabs << "Filter(condition=[";
            std::vector<std::string> conditions;
            for (const auto& cond : scan_plan->conds_) {
                std::stringstream cond_ss;
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
                    if (cond.rhs_val.type == TYPE_INT) {
                        cond_ss << cond.rhs_val.int_val;
                    } else if (cond.rhs_val.type == TYPE_FLOAT) {
                        cond_ss << cond.rhs_val.float_val;
                    } else {
                        cond_ss << "'" << cond.rhs_val.str_val << "'";
                    }
                }
                conditions.push_back(cond_ss.str());
            }
            
            std::sort(conditions.begin(), conditions.end());
            bool first = true;
            for (const auto& cond : conditions) {
                if (!first) ss << ",";
                ss << cond;
                first = false;
            }
            ss << "])" << std::endl;
            
            // 输出Scan节点
            ss << tabs << "\tScan(table=" << scan_plan->tab_name_ << ")" << std::endl;
        } else {
            // 没有过滤条件，直接输出Scan
            ss << tabs << "Scan(table=" << scan_plan->tab_name_ << ")" << std::endl;
        }
        return;
    }
    
    // 其他节点类型的处理
    ss << tabs << "Unknown plan type: " << plan->tag << std::endl;
}

std::unique_ptr<RmRecord> ExplainExecutor::Next() {
    if (!executed_) {
        // 创建包含EXPLAIN结果的记录
        std::stringstream ss;
        
        print_plan_tree(plan_->plan_, ss);


        // 保存结果
        result_ = ss.str();
        
        // 输出到终端
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

