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

//辅助函数,按字典序打印项目列表
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

// 辅助函数,收集JOIN计划中的所有表名
void collect_table_names(std::shared_ptr<JoinPlan> join_plan, std::set<std::string>& table_names,
    const std::map<std::string, std::string>& tab_alias_map = {}) {
    // 使用递归函数收集所有表名
    std::function<void(std::shared_ptr<Plan>)> collect = [&](std::shared_ptr<Plan> p) {
        if (!p) return;
        
        if (auto scan = std::dynamic_pointer_cast<ScanPlan>(p)) {
            std::string tab_name = scan->tab_name_;
            auto it = tab_alias_map.find(tab_name);
            if (it != tab_alias_map.end()) {
                table_names.insert(it->second); // 使用别名
            } else {
                table_names.insert(tab_name); // 使用原表名
            }
        } 
        else if (auto sub_join = std::dynamic_pointer_cast<JoinPlan>(p)) {
            collect(sub_join->left_);
            collect(sub_join->right_);
        }
        else if (auto proj = std::dynamic_pointer_cast<ProjectionPlan>(p)) {
            // 当遇到投影节点时，继续查找其子计划
            collect(proj->subplan_);
        }
    };
    // 从JOIN的左右子树开始收集
    collect(join_plan->left_);
    collect(join_plan->right_);
}


void print_plan_tree(std::shared_ptr<Plan> plan, std::stringstream& ss, 
    int indent = 0,const std::map<std::string, std::string>& tab_alias_map = {}) {
    if(tab_alias_map.empty()) std::cout<<"tab_alias_map为空"<<std::endl;
    else{
        for(auto it : tab_alias_map) std::cout<<it.first<<" "<<it.second<<std::endl;
    }
    std::string tabs(indent, '\t');
    
    if (!plan) return;

    // 处理EXPLAIN和SELECT节点
    if (plan->tag == T_Explain || plan->tag == T_select) {
        if (plan->tag == T_Explain) {
            auto explain_plan = std::dynamic_pointer_cast<ExplainPlan>(plan);
            if (explain_plan && explain_plan->plan_) {
                print_plan_tree(explain_plan->plan_, ss, indent,tab_alias_map);
            }
        } else {
            auto dml_plan = std::dynamic_pointer_cast<DMLPlan>(plan);
            if (dml_plan && dml_plan->subplan_) {
                print_plan_tree(dml_plan->subplan_, ss, indent,tab_alias_map);
            }
        }
        return;
    }

     // 处理投影节点
     if (plan->tag == T_Projection) {
        auto proj_plan = std::dynamic_pointer_cast<ProjectionPlan>(plan);
        ss << tabs << "Project(columns=[";
        
        // select *直接输出*
        if (proj_plan->is_select_star_) {
            ss << "*";
        } else {
            // 收集并排序列名
            std::vector<std::string> column_names;
            for (const auto& col : proj_plan->sel_cols_) {
                std::string tab_name = col.tab_name;
                std::cout<<"DEBUG:Project原表名为:"<<tab_name<<std::endl;
                //别名转换
                auto it = tab_alias_map.find(tab_name);
                if (it != tab_alias_map.end()) {
                    tab_name = it->second; // 使用别名
                }
                std::cout<<"DEBUG:Project转换后的表名为:"<<tab_name<<std::endl;
                column_names.push_back(tab_name + "." + col.col_name);
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
            print_plan_tree(proj_plan->subplan_, ss, indent + 1,tab_alias_map);
        }
        return;
    }
    
    // 处理连接节点
    if (plan->tag == T_NestLoop) {
        auto join_plan = std::dynamic_pointer_cast<JoinPlan>(plan);
        
        // 收集所有表名并排序
        std::set<std::string> table_names;
        collect_table_names(join_plan, table_names, tab_alias_map);

        // 创建一个原始表名数组，用于按字母序输出
        std::vector<std::string> original_tables;

        // 从别名映射回原始表名
        for (const auto& table : table_names) {
            bool found = false;
            for (const auto& [original_table, alias] : tab_alias_map) {
                if (alias == table) {
                    original_tables.push_back(original_table);
                    found = true;
                    break;
                }
            }
            // 如果没有找到别名映射，表示该表没有别名，直接使用表名
            if (!found) {
                original_tables.push_back(table);
            }
        }

        // 对原始表名进行排序
        std::sort(original_tables.begin(), original_tables.end());
        
        // 生成JOIN节点
        ss << tabs << "Join(tables=[";
        bool first = true;
        for (const auto& table : original_tables) {
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
            // 获取左侧列表名，使用别名
            std::string lhs_table = cond.lhs_col.tab_name;
            auto lhs_it = tab_alias_map.find(lhs_table);
            if (lhs_it != tab_alias_map.end()) {
                lhs_table = lhs_it->second; 
            } 
            // 获取右侧列表名，使用别名
            std::string rhs_table = cond.rhs_col.tab_name;
            auto rhs_it = tab_alias_map.find(rhs_table);
            if (rhs_it != tab_alias_map.end()) {
                rhs_table = rhs_it->second; 
            }
            
            cond_ss << lhs_table << "." << cond.lhs_col.col_name << "=";
            cond_ss << rhs_table << "." << cond.rhs_col.col_name;
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
        
    // 将左右子节点封装成pair<优先级字符串, 节点指针>，存入vector
    std::vector<std::pair<std::string, std::shared_ptr<Plan>>> ordered_child_nodes;

    // 确定每个节点的类型和排序优先级
    auto determine_node_priority = [](std::shared_ptr<Plan> node) -> std::string {
        // 基础优先级
        std::string prefix;
        // 附加表名信息，用于同类型节点排序
        std::string table_info = "";
        
        // 特殊处理Scan节点
        if (auto scan = std::dynamic_pointer_cast<ScanPlan>(node)) {
            if (!scan->conds_.empty()) {
                prefix = "A_Filter"; // Filter节点优先级最高
            } else {
                prefix = "D_Scan"; // 普通Scan节点优先级最低
            }
            // 添加表名信息
            table_info = "_" + scan->tab_name_;
        }
        // 处理Project节点,提取其底层表名
        else if (auto proj = std::dynamic_pointer_cast<ProjectionPlan>(node)) {
            prefix = "C_Project";
            
            // 递归获取Project节点底层的表名
            std::function<std::string(std::shared_ptr<Plan>)> get_underlying_table = 
                [&get_underlying_table](std::shared_ptr<Plan> p) -> std::string {
                    if (!p) return "";
                    
                    if (auto scan = std::dynamic_pointer_cast<ScanPlan>(p)) {
                        return scan->tab_name_;
                    } else if (auto sub_proj = std::dynamic_pointer_cast<ProjectionPlan>(p)) {
                        return get_underlying_table(sub_proj->subplan_);
                    } else {
                        return ""; // 无表名信息
                    }
                };
                
            // 获取并添加表名信息
            table_info = "_" + get_underlying_table(proj->subplan_);
        }
        // 处理Join节点
        else if (node->tag == T_NestLoop) {
            prefix = "B_Join";
            // Join节点无需排序
        }
        // 其他节点
        else {
            prefix = "Z_Other";
        }
        
        return prefix + table_info;
    };

    // 封装左右子节点
    ordered_child_nodes.push_back({determine_node_priority(join_plan->left_), join_plan->left_});
    ordered_child_nodes.push_back({determine_node_priority(join_plan->right_), join_plan->right_});

    // 按优先级排序
    std::sort(ordered_child_nodes.begin(), ordered_child_nodes.end());

    // 依次处理排序后的节点
    for (const auto& [priority, node] : ordered_child_nodes) {
        if (auto scan = std::dynamic_pointer_cast<ScanPlan>(node)) {
            if (!scan->conds_.empty()) {
                // 输出Filter节点
                ss << tabs << "\tFilter(condition=[";
                std::vector<std::string> filter_conds;
                for (const auto& cond : scan->conds_) {
                    std::stringstream cond_ss;
                    std::string tab_name = scan->tab_name_;
                    auto it = tab_alias_map.find(tab_name);
                    for (const auto& [original_table, alias] : tab_alias_map) {
                        if (original_table == tab_name) {
                            tab_name = alias; // 使用别名
                            break;
                        }
                    }
                    cond_ss << tab_name << "." << cond.lhs_col.col_name;
                    
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
                    }else {
                        // 如果右侧也是列，也应用别名映射
                        std::string rhs_tab_name = cond.rhs_col.tab_name;
                        for (const auto& [original_table, alias] : tab_alias_map) {
                            if (original_table == rhs_tab_name) {
                                rhs_tab_name = alias; // 使用别名
                                break;
                            }
                        }
                        cond_ss << rhs_tab_name << "." << cond.rhs_col.col_name;
                    }
                    filter_conds.push_back(cond_ss.str());
                }
                
                std::sort(filter_conds.begin(), filter_conds.end());
                bool first = true;
                for (const auto& cond : filter_conds) {
                    if (!first) ss << ",";
                    ss << cond;
                    first = false;
                }
                ss << "])" << std::endl;
                
                // 输出Scan节点
                ss << tabs << "\t\tScan(table=" << scan->tab_name_ << ")" << std::endl;
            } else {
                // 直接输出Scan节点
                ss << tabs << "\tScan(table=" << scan->tab_name_ << ")" << std::endl;
            }
        } else {
            // 其他类型节点递归处理
            print_plan_tree(node, ss, indent + 1,tab_alias_map);
        }
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
        std::cout << "DEBUG: 开始生成EXPLAIN输出" << std::endl;
        print_plan_tree(plan_->plan_, ss, 0, tab_alias_map_);
        // 保存结果
        result_ = ss.str();
        // 输出到终端
        std::cerr << "\n======= EXPLAIN QUERY PLAN =======\n";
        std::cerr << result_;
        std::cerr << "=================================\n\n";
        
        // 创建记录
        auto rec = std::make_unique<RmRecord>(4096);
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
    //beginTuple() 中调用,实际代码已在 Next() 中实现
}

std::unique_ptr<AbstractExecutor> ExplainPlan::get_executor(Context *context) {
    std::cout << "DEBUG: 创建ExplainExecutor,别名映射大小: " << tab_alias_map.size() << std::endl;
    return std::make_unique<ExplainExecutor>(this, context);
}

