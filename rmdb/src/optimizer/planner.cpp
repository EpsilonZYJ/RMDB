/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "planner.h"

#include <memory>

#include "execution/executor_delete.h"
#include "execution/executor_index_scan.h"
#include "execution/executor_insert.h"
#include "execution/executor_nestedloop_join.h"
#include "execution/executor_projection.h"
#include "execution/executor_seq_scan.h"
#include "execution/executor_update.h"
#include "index/ix.h"
#include "record_printer.h"
//#include "execution/executor_filter.h" 
#include <set>
#include <cmath>
// 目前的索引匹配规则为：完全匹配索引字段，且全部为单点查询，不会自动调整where条件的顺序
bool Planner::get_index_cols(std::string tab_name, std::vector<Condition> curr_conds, std::vector<std::string>& index_col_names) {
    index_col_names.clear();
    for(auto& cond: curr_conds) {
        if(cond.is_rhs_val && cond.op == OP_EQ && cond.lhs_col.tab_name.compare(tab_name) == 0)
            index_col_names.push_back(cond.lhs_col.col_name);
    }
    TabMeta& tab = sm_manager_->db_.get_table(tab_name);
    if(tab.is_index(index_col_names)) return true;
    return false;
}

/**
 * @brief 表算子条件谓词生成
 *
 * @param conds 条件
 * @param tab_names 表名
 * @return std::vector<Condition>
 */
 std::vector<Condition> pop_conds(std::vector<Condition> &conds, std::string tab_names) {
    // auto has_tab = [&](const std::string &tab_name) {
    //     return std::find(tab_names.begin(), tab_names.end(), tab_name) != tab_names.end();
    // };
    std::vector<Condition> solved_conds;
    auto it = conds.begin();
    while (it != conds.end()) {
        if ((tab_names.compare(it->lhs_col.tab_name) == 0 && it->is_rhs_val) || (it->lhs_col.tab_name.compare(it->rhs_col.tab_name) == 0)) {
            solved_conds.emplace_back(std::move(*it));
            it = conds.erase(it);
        } else {
            it++;
        }
    }
    return solved_conds;
}


int push_conds(Condition *cond, std::shared_ptr<Plan> plan)
{
    if(auto x = std::dynamic_pointer_cast<ScanPlan>(plan))
    {
        if(x->tab_name_.compare(cond->lhs_col.tab_name) == 0) {
            return 1;
        } else if(x->tab_name_.compare(cond->rhs_col.tab_name) == 0){
            return 2;
        } else {
            return 0;
        }
    }
    else if(auto x = std::dynamic_pointer_cast<JoinPlan>(plan))
    {
        int left_res = push_conds(cond, x->left_);
        // 条件已经下推到左子节点
        if(left_res == 3){
            return 3;
        }
        int right_res = push_conds(cond, x->right_);
        // 条件已经下推到右子节点
        if(right_res == 3){
            return 3;
        }
        // 左子节点或右子节点有一个没有匹配到条件的列
        if(left_res == 0 || right_res == 0) {
            return left_res + right_res;
        }
        // 左子节点匹配到条件的右边
        if(left_res == 2) {
            // 需要将左右两边的条件变换位置
            std::map<CompOp, CompOp> swap_op = {
                {OP_EQ, OP_EQ}, {OP_NE, OP_NE}, {OP_LT, OP_GT}, {OP_GT, OP_LT}, {OP_LE, OP_GE}, {OP_GE, OP_LE},
            };
            std::swap(cond->lhs_col, cond->rhs_col);
            cond->op = swap_op.at(cond->op);
        }
        x->conds_.emplace_back(std::move(*cond));
        return 3;
    }
    return false;
}

std::shared_ptr<Plan> pop_scan(int *scantbl, std::string table, std::vector<std::string> &joined_tables, 
                std::vector<std::shared_ptr<Plan>> plans)
{
    for (size_t i = 0; i < plans.size(); i++) {
        auto x = std::dynamic_pointer_cast<ScanPlan>(plans[i]);
        if(x->tab_name_.compare(table) == 0)
        {
            scantbl[i] = 1;
            joined_tables.emplace_back(x->tab_name_);
            return plans[i];
        }
    }
    return nullptr;
}


std::shared_ptr<Query> Planner::logical_optimization(std::shared_ptr<Query> query, Context *context)
{
    
    //TODO 实现逻辑优化规则
    //1. 谓词下推 - 常量传播优化
    predicate_pushdown(query, context);
    
    // 2. 投影下推
    projection_pushdown(query, context);
    return query;
}

void Planner::predicate_pushdown(std::shared_ptr<Query> query, Context *context)
{
    // 常量传播优化 - 增强谓词下推效果
    std::map<TabCol, Value> map;

    //找出形如"列=常量"的等值条件
    for (auto &cond: query->conds) {
        if (cond.is_rhs_val && cond.op == OP_EQ) {
            map[cond.lhs_col] = cond.rhs_val; 
        }
    }

    //将"列1=列2"和"列2=常量"推导出"列1=常量"
    bool changed;
    do {
        changed = false;
        for (auto it = query->conds.begin(); it != query->conds.end(); ++it) {
            // 处理"列1=列2"形式的条件
            if (!it->is_rhs_val && it->op == OP_EQ) {
                // 检查右侧列是否有对应常量
                auto map_it = map.find(it->rhs_col);
                if (map_it != map.end()) {
                    // 创建新条件: 列1=常量
                    Condition new_cond;
                    new_cond.lhs_col = it->lhs_col;
                    new_cond.op = OP_EQ;
                    new_cond.is_rhs_val = true;
                    new_cond.rhs_val = map_it->second;
                    
                    // 检查是否已有此条件
                    bool exists = false;
                    for (const auto& c : query->conds) {
                        if (c.is_rhs_val && c.op == OP_EQ && 
                            c.lhs_col.tab_name == new_cond.lhs_col.tab_name && 
                            c.lhs_col.col_name == new_cond.lhs_col.col_name) {
                            exists = true;
                            break;
                        }
                    }
                    
                    if (!exists) {
                        query->conds.push_back(new_cond);
                        map.emplace(new_cond.lhs_col, new_cond.rhs_val);
                        changed = true;
                    }
                }
                
                // 检查左侧列是否有对应常量
                map_it = map.find(it->lhs_col);
                if (map_it != map.end()) {
                    // 创建新条件: 列2=常量
                    Condition new_cond;
                    new_cond.lhs_col = it->rhs_col;
                    new_cond.op = OP_EQ;
                    new_cond.is_rhs_val = true;
                    new_cond.rhs_val = map_it->second;
                    
                    // 检查是否已有此条件
                    bool exists = false;
                    for (const auto& c : query->conds) {
                        if (c.is_rhs_val && c.op == OP_EQ && 
                            c.lhs_col.tab_name == new_cond.lhs_col.tab_name && 
                            c.lhs_col.col_name == new_cond.lhs_col.col_name) {
                            exists = true;
                            break;
                        }
                    }
                    
                    if (!exists) {
                        query->conds.push_back(new_cond);
                        map.emplace(new_cond.lhs_col, new_cond.rhs_val);
                        changed = true;
                    }
                }
            }
        }
    } while (changed); // 重复直到没有新条件产生

    // // 将连接条件和过滤条件分离，确保过滤条件尽可能下推
    std::vector<Condition> join_conds;
    // //std::vector<Condition> filter_conds;
    
    for (const auto& cond : query->conds) {
        if (!cond.is_rhs_val) {
            // 检查是否是连接条件（涉及两个不同的表）
            if (cond.lhs_col.tab_name != cond.rhs_col.tab_name) {
                join_conds.push_back(cond);
                continue;
            }
        }
    //     // 这是过滤条件，应该尽可能下推
    //     filter_conds.push_back(cond);
    }
    
    // 更新query中的条件，先放过滤条件，再放连接条件
    //query->conds.clear();
    //query->conds.insert(query->conds.end(), filter_conds.begin(), filter_conds.end());
    query->conds.insert(query->conds.end(), join_conds.begin(), join_conds.end());
}

void Planner::projection_pushdown(std::shared_ptr<Query> query, Context *context)
{
    // 收集查询中所有需要的列
    std::unordered_set<std::string> used_cols;
    
    // 从SELECT子句收集
    for (const auto& col : query->cols) {
        std::string col_id = col.tab_name + "." + col.col_name;
        used_cols.insert(col_id);
    }
    
    // 从WHERE子句条件中收集
    for (const auto& cond : query->conds) {
        std::string lhs_id = cond.lhs_col.tab_name + "." + cond.lhs_col.col_name;
        used_cols.insert(lhs_id);
        
        if (!cond.is_rhs_val) {
            std::string rhs_id = cond.rhs_col.tab_name + "." + cond.rhs_col.col_name;
            used_cols.insert(rhs_id);
        }
    }
    
    // 从ORDER BY子句收集
    auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    if (x && x->has_sort) {
        std::string tab_name;
        // 找出排序列所属的表
        for (const auto& table : query->tables) {
            const auto& tab_meta = sm_manager_->db_.get_table(table);
            for (const auto& col : tab_meta.cols) {
                if (col.name == x->order->cols->col_name) {
                    tab_name = table;
                    break;
                }
            }
            if (!tab_name.empty()) break;
        }
        
        if (!tab_name.empty()) {
            std::string col_id = tab_name + "." + x->order->cols->col_name;
            used_cols.insert(col_id);
        }
    }
    
}

size_t Planner::get_table_cardinality(const std::string& tab_name) {
    // 通过系统管理器获取表元数据
    TabMeta& tab = sm_manager_->db_.get_table(tab_name);
    
    // 获取表的记录数
    RmFileHandle* fh = sm_manager_->fhs_.at(tab_name).get();
    if (fh) {
        return fh->get_file_hdr().num_pages;  // 使用页数作为近似估计
    }
    return 1;
}

size_t Planner::estimate_join_size(
    const std::vector<size_t>& joined_indices,
    size_t new_index,
    const std::vector<std::string>& tables,
    const std::vector<size_t>& cardinalities,
    const std::vector<Condition>& conds) 
{
    // 计算已连接表的总大小
    size_t joined_size = 1;
    for (auto idx : joined_indices) {
        joined_size *= cardinalities[idx];
    }
    
    // 初始结果为笛卡尔积
    size_t result = joined_size * cardinalities[new_index];
    
    // 检查连接条件来调整估计
    for (const auto& cond : conds) {
        if (cond.is_rhs_val) continue; // 跳过常量条件
        
        bool lhs_in_new = (cond.lhs_col.tab_name == tables[new_index]);
        bool rhs_in_new = (cond.rhs_col.tab_name == tables[new_index]);
        
        bool lhs_in_joined = false;
        bool rhs_in_joined = false;
        
        for (auto idx : joined_indices) {
            if (cond.lhs_col.tab_name == tables[idx]) lhs_in_joined = true;
            if (cond.rhs_col.tab_name == tables[idx]) rhs_in_joined = true;
        }
        
        // 如果条件连接了新表和已连接表
        if ((lhs_in_new && rhs_in_joined) || (lhs_in_joined && rhs_in_new)) {
            // 计算选择率
            double selectivity = calculate_join_selectivity(
                cond.lhs_col.tab_name, cond.lhs_col.col_name,
                cond.rhs_col.tab_name, cond.rhs_col.col_name,
                cond.op
            );
            
            // 应用选择率
            result = static_cast<size_t>(result * selectivity);
        }
    }
    
    // 确保结果至少为1
    return std::max<size_t>(1, result);
}

double Planner::calculate_join_selectivity(
    const std::string& lhs_tab, const std::string& lhs_col,
    const std::string& rhs_tab, const std::string& rhs_col,
    CompOp op) 
{
    // 获取两个表的大小
    size_t lhs_size = get_table_cardinality(lhs_tab);
    size_t rhs_size = get_table_cardinality(rhs_tab);
    
    // 估计不同值的数量 (NDV)
    size_t lhs_ndv = estimate_ndv(lhs_tab, lhs_col);
    size_t rhs_ndv = estimate_ndv(rhs_tab, rhs_col);
    
    // 对于等值连接，选择率 = 1 / max(NDV)
    if (op == OP_EQ) {
        return 1.0 / std::max<double>(lhs_ndv, 1.0);
    } 
    // 对于非等值连接，使用更保守的估计
    else {
        return 0.3;  // 可以基于具体操作符进行更精细的估计
    }
}

// 估计列的不同值数量
size_t Planner::estimate_ndv(const std::string& tab_name, const std::string& col_name) {
    // 获取表大小
    size_t table_size = get_table_cardinality(tab_name);
    
    // 根据经验公式估计NDV
    return std::max<size_t>(1, static_cast<size_t>(sqrt(table_size)));
}

std::shared_ptr<Plan> Planner::physical_optimization(std::shared_ptr<Query> query, Context *context)
{
    std::cout << "DEBUG: physical_optimization 开始" << std::endl;
    std::shared_ptr<Plan> plan = make_one_rel(query);
    std::cout << "DEBUG: make_one_rel 完成" << std::endl;
    // 其他物理优化

    // 处理orderby
    plan = generate_sort_plan(query, std::move(plan)); 
    std::cout << "DEBUG: generate_sort_plan 完成" << std::endl;
    return plan;
}

std::shared_ptr<Plan> Planner::make_one_rel(std::shared_ptr<Query> query)
{
    std::cout << "DEBUG: make_one_rel 开始" << std::endl;
    auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    std::vector<std::string> tables = query->tables;
    std::cout << "DEBUG: 处理的表数量: " << tables.size() << std::endl;
    // 1. 为每个表创建扫描执行器
    std::vector<std::shared_ptr<Plan>> table_scan_executors(tables.size());
    std::vector<size_t> table_cardinalities(tables.size());
    
    // 创建扫描计划 - 分离过滤条件和表扫描
    for (size_t i = 0; i < tables.size(); i++) {
        //应用谓词下推 - 提取与表相关的条件
        auto curr_conds = pop_conds(query->conds, tables[i]);
        std::cout <<"DEBUG: 谓词下推完成"<< std::endl;
        // // 检查是否有过滤条件
        // bool has_filters = !curr_conds.empty();
        
        // 检查是否可以使用索引
        std::vector<std::string> index_col_names;
        bool index_exist = get_index_cols(tables[i], curr_conds, index_col_names);
        
        // 创建基本的扫描计划（不带条件）
        std::shared_ptr<Plan> scan_plan;
        if (!index_exist) {
            index_col_names.clear();
            scan_plan = std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, tables[i], 
                                                std::vector<Condition>(), index_col_names);
        } else {
            scan_plan = std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, tables[i], 
                                                std::vector<Condition>(), index_col_names);
        }
        std::cout <<"DEBUG: 创建基本扫描计划完成"<< std::endl;
        
        //将scan_plan保存到table_scan_executors数组中
        table_scan_executors[i] = scan_plan;

        // 获取表的基数(行数)
        std::cout << "DEBUG: 获取表 " << tables[i] << " 的基数" << std::endl;
        table_cardinalities[i] = get_table_cardinality(tables[i]);
        std::cout <<"DEBUG: 获取表基数完成"<< std::endl;
    }
    std::cout <<"DEBUG: 创建扫描计划完成"<< std::endl;
    // 只有一个表，不需要连接
    if (tables.size() == 1) {
        return table_scan_executors[0];
    }
    
    // 2. 连接顺序优化 - 贪心算法
    // 使用vector跟踪已加入连接计划和未加入的表
    std::vector<bool> used(tables.size(), false);
    std::vector<size_t> join_order;
    
    // 首先选择有过滤条件的表或基数最小的表
    size_t min_idx = 0;
    //bool has_filter = false;

    for (size_t i = 0; i < tables.size(); i++) {
        // 优先选择有过滤条件的表，或者在没有过滤条件的情况下选择基数最小的表
        if ( table_cardinalities[i] < table_cardinalities[min_idx]) {
            min_idx = i;
            //has_filter = table_has_filter;
        }
    }
    
    join_order.push_back(min_idx);
    used[min_idx] = true;
    std::cout <<"DEBUG: 基数最小的表查找完成"<< std::endl;
    // 选择基数第二小的表或连接后结果最小的表
    size_t second_best_idx = SIZE_MAX;
    size_t min_result_size = SIZE_MAX;
    
    for (size_t i = 0; i < tables.size(); i++) {
        if (used[i]) continue;
        
        // 估计与第一个表连接后的大小
        std::vector<size_t> current_joined = {min_idx};
        size_t result_size = estimate_join_size(current_joined, i, tables, table_cardinalities, query->conds);
        std::cout <<"DEBUG: 估计连接大小完成"<< std::endl;
        if (result_size < min_result_size) {
            min_result_size = result_size;
            second_best_idx = i;
        }
    }
    
    join_order.push_back(second_best_idx);
    used[second_best_idx] = true;
    
    // 根据最小结果集选择后续表
    while (join_order.size() < tables.size()) {
        size_t best_idx = SIZE_MAX;
        size_t min_result_size = SIZE_MAX;
        
        for (size_t i = 0; i < tables.size(); i++) {
            if (used[i]) continue;
            
            // 估计加入当前已连接表的结果大小
            size_t result_size = estimate_join_size(join_order, i, tables, table_cardinalities, query->conds);
            std::cout <<"DEBUG: 估计加入当前已连接表的大小完成"<< std::endl;
            if (result_size < min_result_size) {
                min_result_size = result_size;
                best_idx = i;
            }
        }
        
        join_order.push_back(best_idx);
        used[best_idx] = true;
    }
    
    // 3. 根据优化后的顺序构建左深树
    std::shared_ptr<Plan> join_plan = table_scan_executors[join_order[0]];
    
    for (size_t i = 1; i < join_order.size(); i++) {
        // 找出连接条件
        std::vector<Condition> join_conds;
        
        for (auto it = query->conds.begin(); it != query->conds.end();) {
            if (it->is_rhs_val) {
                ++it;
                continue;
            }
            
            // 检查是否连接当前表和已连接表
            bool connects_current = (it->lhs_col.tab_name == tables[join_order[i]] ||
                                    it->rhs_col.tab_name == tables[join_order[i]]);
            
            bool connects_joined = false;
            for (size_t j = 0; j < i; j++) {
                if (it->lhs_col.tab_name == tables[join_order[j]] ||
                    it->rhs_col.tab_name == tables[join_order[j]]) {
                    connects_joined = true;
                    break;
                }
            }
            
            if (connects_current && connects_joined) {
                join_conds.push_back(*it);
                it = query->conds.erase(it);
            } else {
                ++it;
            }
        }
        
       // 选择连接算法
        PlanTag join_type = T_NestLoop; // 使用嵌套循环连接

        // 创建连接计划
        join_plan = std::make_shared<JoinPlan>(
            join_type, 
            join_plan,
            table_scan_executors[join_order[i]],
            join_conds
        );
    }
    std::cout <<"DEBUG: 构建左深树完成"<< std::endl;
    // 处理剩余的条件
    for (auto& cond : query->conds) {
        push_conds(&cond, join_plan);
    }
    
    return join_plan;
}

void Planner::explain_plan(std::shared_ptr<Plan> plan, std::ostream& os, int indent) {
    std::string indent_str(indent, '\t');
    
    if (!plan) {
        os << indent_str << "NULL plan" << std::endl;
        return;
    }

    switch (plan->tag) {
        case T_select: {
            if (auto dml_plan = std::dynamic_pointer_cast<DMLPlan>(plan)) {
                // 跳过SELECT关键字，直接显示下层节点
                this->explain_plan(dml_plan->subplan_, os, indent);
            }
            break;
        }
        case T_SeqScan: {
            if (auto scan_plan = std::dynamic_pointer_cast<ScanPlan>(plan)) {
                // 分离条件和扫描
                if (!scan_plan->conds_.empty()) {
                    // 先输出Project节点(如果适用)
                    os << indent_str << "Project(columns=[";
                    // 按字母序排序列
                    std::vector<std::string> columns;
                    // 这里需要获取表的所有列
                    // 假设从scan_plan中可以获取
                    columns.push_back(scan_plan->tab_name_ + ".*");
                    os << columns[0];
                    for (size_t i = 1; i < columns.size(); ++i) {
                        os << "," << columns[i];
                    }
                    os << "])" << std::endl;
                    
                    // 输出Filter节点
                    os << indent_str << "\tFilter(condition=[";
                    // 按字典序排序条件
                    std::vector<std::string> conditions;
                    for (const auto& cond : scan_plan->conds_) {
                        std::stringstream cond_ss;
                        cond_ss << scan_plan->tab_name_ << "." << cond.lhs_col.col_name;
                        
                        switch (cond.op) {
                            case OP_EQ: cond_ss << "="; break;
                            case OP_NE: cond_ss << "!="; break;
                            case OP_LT: cond_ss << "<"; break;
                            case OP_LE: cond_ss << "<="; break;
                            case OP_GT: cond_ss << ">"; break;
                            case OP_GE: cond_ss << ">="; break;
                            default: cond_ss << "?"; break;
                        }
                        
                        if (cond.is_rhs_val) {
                            if (cond.rhs_val.type == TYPE_INT) {
                                cond_ss << cond.rhs_val.int_val;
                            } else if (cond.rhs_val.type == TYPE_FLOAT) {
                                cond_ss << cond.rhs_val.float_val;
                            } else if (cond.rhs_val.type == TYPE_STRING) {
                                cond_ss << cond.rhs_val.str_val;
                            }
                        } else {
                            cond_ss << cond.rhs_col.col_name;
                        }
                        conditions.push_back(cond_ss.str());
                    }
                    
                    // 按字典序排序条件
                    std::sort(conditions.begin(), conditions.end());
                    
                    os << conditions[0];
                    for (size_t i = 1; i < conditions.size(); ++i) {
                        os << "," << conditions[i];
                    }
                    os << "])" << std::endl;
                    
                    // 输出Scan节点
                    os << indent_str << "\t\tScan(table=" << scan_plan->tab_name_ << ")" << std::endl;
                } else {
                    // 无条件，直接输出Scan节点
                    os << indent_str << "Scan(table=" << scan_plan->tab_name_ << ")" << std::endl;
                }
            }
            break;
        }
        case T_IndexScan: {
            if (auto scan_plan = std::dynamic_pointer_cast<ScanPlan>(plan)) {
                // 使用索引扫描，但格式仍为Scan
                os << indent_str << "Scan(table=" << scan_plan->tab_name_;
                os << ", index=" << scan_plan->index_col_names_.front() << ")" << std::endl;
            }
            break;
        }
        case T_NestLoop: {
            if (auto join_plan = std::dynamic_pointer_cast<JoinPlan>(plan)) {
                // 收集所有表名 - 完整实现
                std::vector<std::string> tables;
                
                // 通过递归函数收集所有涉及的表
                std::function<void(std::shared_ptr<Plan>, std::vector<std::string>&)> collect_tables = 
                    [&](std::shared_ptr<Plan> subplan, std::vector<std::string>& table_list) {
                        if (!subplan) return;
                        
                        if (auto scan = std::dynamic_pointer_cast<ScanPlan>(subplan)) {
                            // 扫描节点直接添加表名
                            table_list.push_back(scan->tab_name_);
                        } else if (auto sub_join = std::dynamic_pointer_cast<JoinPlan>(subplan)) {
                            // 递归处理连接的左右子树
                            collect_tables(sub_join->left_, table_list);
                            collect_tables(sub_join->right_, table_list);
                        } else if (auto proj = std::dynamic_pointer_cast<ProjectionPlan>(subplan)) {
                            // 递归处理投影的子树
                            collect_tables(proj->subplan_, table_list);
                        } else if (auto sort = std::dynamic_pointer_cast<SortPlan>(subplan)) {
                            // 递归处理排序的子树
                            collect_tables(sort->subplan_, table_list);
                        } else if (auto dml = std::dynamic_pointer_cast<DMLPlan>(subplan)) {
                            // 递归处理DML的子树
                            if (dml->subplan_) {
                                collect_tables(dml->subplan_, table_list);
                            }
                        }
                    };
                
                // 从左右子树收集表名
                collect_tables(join_plan->left_, tables);
                collect_tables(join_plan->right_, tables);
                
                // 移除重复表名
                std::sort(tables.begin(), tables.end());
                tables.erase(std::unique(tables.begin(), tables.end()), tables.end());
                
                // 生成输出
                os << indent_str << "Join(tables=[";
                if (!tables.empty()) {
                    os << tables[0];
                    for (size_t i = 1; i < tables.size(); ++i) {
                        os << "," << tables[i];
                    }
                }
                os << "], condition=[";
                
                // 收集并排序连接条件
                std::vector<std::string> join_conditions;
                for (const auto& cond : join_plan->conds_) {
                    std::stringstream cond_ss;
                    cond_ss << cond.lhs_col.tab_name << "." << cond.lhs_col.col_name;
                    cond_ss << "=";
                    cond_ss << cond.rhs_col.tab_name << "." << cond.rhs_col.col_name;
                    join_conditions.push_back(cond_ss.str());
                }
                
                std::sort(join_conditions.begin(), join_conditions.end());
                
                if (!join_conditions.empty()) {
                    os << join_conditions[0];
                    for (size_t i = 1; i < join_conditions.size(); ++i) {
                        os << "," << join_conditions[i];
                    }
                }
                os << "])" << std::endl;
                
                // 继续处理子节点
                this->explain_plan(join_plan->left_, os, indent + 1);
                this->explain_plan(join_plan->right_, os, indent + 1);
            }
            break;
        }
        case T_Projection: {
            if (auto proj_plan = std::dynamic_pointer_cast<ProjectionPlan>(plan)) {
                os << indent_str << "Project(columns=[";
                
                // 检查是否选择所有列
                bool is_select_all = false;

        // 检查是否是SELECT *的更准确方法
        if (proj_plan->sel_cols_.empty()) {
            is_select_all = true;
        } else {
            // 获取所有涉及的表
            std::set<std::string> tables;
            for (const auto& col : proj_plan->sel_cols_) {
                if (!col.tab_name.empty()) {
                    tables.insert(col.tab_name);
                }
            }
            
            // 检查是否包含了每个表的所有列
            bool all_columns_selected = true;
            for (const auto& tab : tables) {
                // 获取表的列数
                size_t table_column_count = sm_manager_->db_.get_table(tab).cols.size();
                
                // 计算选择的该表列数
                size_t selected_column_count = 0;
                for (const auto& col : proj_plan->sel_cols_) {
                    if (col.tab_name == tab) {
                        selected_column_count++;
                    }
                }
                
                if (selected_column_count < table_column_count) {
                    all_columns_selected = false;
                    break;
                }
            }
            
            is_select_all = all_columns_selected;
        }
                
                if (is_select_all) {
                    os << "*";
                } else {
                    // 排序和显示特定列的原有代码
                    std::vector<std::string> columns;
                    for (const auto& col : proj_plan->sel_cols_) {
                        std::string col_name;
                        if (!col.tab_name.empty()) {
                            col_name = col.tab_name + "." + col.col_name;
                        } else {
                            col_name = col.col_name;
                        }
                        columns.push_back(col_name);
                    }
                    
                    // 按字母序排序
                    std::sort(columns.begin(), columns.end());
                    
                    if (!columns.empty()) {
                        os << columns[0];
                        for (size_t i = 1; i < columns.size(); ++i) {
                            os << "," << columns[i];
                        }
                    }
                }
                
                os << "])" << std::endl;
                
                // 处理子节点
                this->explain_plan(proj_plan->subplan_, os, indent + 1);
            }
            break;
        }
        case T_Explain: {
            if (auto explain_plan = std::dynamic_pointer_cast<ExplainPlan>(plan)) {
                // 直接处理子节点
                this->explain_plan(explain_plan->plan_, os, indent);
            }
            break;
        }
        default:
            os << indent_str << "Unknown plan type: " << plan->tag << std::endl;
            break;
    }
}

std::shared_ptr<Plan> Planner::generate_sort_plan(std::shared_ptr<Query> query, std::shared_ptr<Plan> plan)
{
    auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    if(!x->has_sort) {
        return plan;
    }
    std::vector<std::string> tables = query->tables;
    std::vector<ColMeta> all_cols;
    for (auto &sel_tab_name : tables) {
        // 这里db_不能写成get_db(), 注意要传指针
        const auto &sel_tab_cols = sm_manager_->db_.get_table(sel_tab_name).cols;
        all_cols.insert(all_cols.end(), sel_tab_cols.begin(), sel_tab_cols.end());
    }
    TabCol sel_col;
    for (auto &col : all_cols) {
        if(col.name.compare(x->order->cols->col_name) == 0 )
        sel_col = {.tab_name = col.tab_name, .col_name = col.name};
    }
    return std::make_shared<SortPlan>(T_Sort, std::move(plan), sel_col, 
                                    x->order->orderby_dir == ast::OrderBy_DESC);
}


/**
 * @brief select plan 生成
 *
 * @param sel_cols select plan 选取的列
 * @param tab_names select plan 目标的表
 * @param conds select plan 选取条件
 */
std::shared_ptr<Plan> Planner::generate_select_plan(std::shared_ptr<Query> query, Context *context) {
    //逻辑优化
    std::cout << "DEBUG: generate_select_plan 开始" << std::endl;
    std::cout << "DEBUG: 开始逻辑优化" << std::endl;
    query = logical_optimization(std::move(query), context);
    
    //处理JOIN ON条件，将它们合并到WHERE条件中
    if (!query->join_conds.empty()) {
        std::cout << "DEBUG: 处理 JOIN ON 条件，数量: " << query->join_conds.size() << std::endl;
        for (const auto& join_cond_set : query->join_conds) {
            // 将JOIN条件添加到WHERE条件列表中
            query->conds.insert(query->conds.end(), join_cond_set.begin(), join_cond_set.end());
        }
        
        // 清空join_conds，防止重复处理
        query->join_conds.clear();
    }
    std::cout << "DEBUG: 开始物理优化" << std::endl;

    //物理优化
    auto sel_cols = query->cols;
    std::shared_ptr<Plan> plannerRoot = physical_optimization(query, context);
    std::cout << "DEBUG: 创建投影计划" << std::endl;
    plannerRoot = std::make_shared<ProjectionPlan>(T_Projection, std::move(plannerRoot), 
                                                        std::move(sel_cols));
    std::cout << "DEBUG: 创建投影计划完成" << std::endl;
    return plannerRoot;
}


std::shared_ptr<Plan> Planner::do_planner(std::shared_ptr<Query> query, Context *context)
{
    bool is_explain = query->is_explain;
    std::shared_ptr<Plan> plannerRoot;
    if (auto x = std::dynamic_pointer_cast<ast::CreateTable>(query->parse)) {
        // create table;
        std::vector<ColDef> col_defs;
        for (auto &field : x->fields) {
            if (auto sv_col_def = std::dynamic_pointer_cast<ast::ColDef>(field)) {
                ColDef col_def = {.name = sv_col_def->col_name,
                                  .type = interp_sv_type(sv_col_def->type_len->type),
                                  .len = sv_col_def->type_len->len};
                col_defs.push_back(col_def);
            } else {
                throw InternalError("Unexpected field type");
            }
        }
        plannerRoot = std::make_shared<DDLPlan>(T_CreateTable, x->tab_name, std::vector<std::string>(), col_defs);
    } else if (auto x = std::dynamic_pointer_cast<ast::DropTable>(query->parse)) {
        // drop table;
        plannerRoot = std::make_shared<DDLPlan>(T_DropTable, x->tab_name, std::vector<std::string>(), std::vector<ColDef>());
    } else if (auto x = std::dynamic_pointer_cast<ast::CreateIndex>(query->parse)) {
        // create index;
        plannerRoot = std::make_shared<DDLPlan>(T_CreateIndex, x->tab_name, x->col_names, std::vector<ColDef>());
    } else if (auto x = std::dynamic_pointer_cast<ast::DropIndex>(query->parse)) {
        // drop index
        plannerRoot = std::make_shared<DDLPlan>(T_DropIndex, x->tab_name, x->col_names, std::vector<ColDef>());
    } else if (auto x = std::dynamic_pointer_cast<ast::InsertStmt>(query->parse)) {
        // insert;
        plannerRoot = std::make_shared<DMLPlan>(T_Insert, std::shared_ptr<Plan>(),  x->tab_name,  
                                                    query->values, std::vector<Condition>(), std::vector<SetClause>());
    } else if (auto x = std::dynamic_pointer_cast<ast::DeleteStmt>(query->parse)) {
        // delete;
        // 生成表扫描方式
        std::shared_ptr<Plan> table_scan_executors;
        // 只有一张表，不需要进行物理优化了
        // int index_no = get_indexNo(x->tab_name, query->conds);
        std::vector<std::string> index_col_names;
        bool index_exist = get_index_cols(x->tab_name, query->conds, index_col_names);
        
        if (index_exist == false) {  // 该表没有索引
            index_col_names.clear();
            table_scan_executors = 
                std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, x->tab_name, query->conds, index_col_names);
        } else {  // 存在索引
            table_scan_executors =
                std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, x->tab_name, query->conds, index_col_names);
        }

        plannerRoot = std::make_shared<DMLPlan>(T_Delete, table_scan_executors, x->tab_name,  
                                                std::vector<Value>(), query->conds, std::vector<SetClause>());
    } else if (auto x = std::dynamic_pointer_cast<ast::UpdateStmt>(query->parse)) {
        // update;
        // 生成表扫描方式
        std::shared_ptr<Plan> table_scan_executors;
        // 只有一张表，不需要进行物理优化了
        // int index_no = get_indexNo(x->tab_name, query->conds);
        std::vector<std::string> index_col_names;
        bool index_exist = get_index_cols(x->tab_name, query->conds, index_col_names);

        if (index_exist == false) {  // 该表没有索引
        index_col_names.clear();
            table_scan_executors = 
                std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, x->tab_name, query->conds, index_col_names);
        } else {  // 存在索引
            table_scan_executors =
                std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, x->tab_name, query->conds, index_col_names);
        }
        plannerRoot = std::make_shared<DMLPlan>(T_Update, table_scan_executors, x->tab_name,
                                                     std::vector<Value>(), query->conds, 
                                                     query->set_clauses);
    } else if (auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse)) {

        std::shared_ptr<plannerInfo> root = std::make_shared<plannerInfo>(x);
        // 生成select语句的查询执行计划
        std::shared_ptr<Plan> projection = generate_select_plan(std::move(query), context);
        plannerRoot = std::make_shared<DMLPlan>(T_select, projection, std::string(), std::vector<Value>(),
                                                    std::vector<Condition>(), std::vector<SetClause>());
    } else {
        throw InternalError("Unexpected AST root");
    }
    if (is_explain) {
        return std::make_shared<ExplainPlan>(T_Explain, plannerRoot);
    }
    std::cout << "DEBUG: doplanner完成" << std::endl;
    return plannerRoot;
}