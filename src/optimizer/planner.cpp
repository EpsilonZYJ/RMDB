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
    #include "execution/executor_semi_join.h" 
    #include <set>
    #include <cmath>

    /**
    * @brief 评估单个索引的匹配度
    * @param index 某一个索引信息
    * @param condition_mapping 条件映射
    * @param conditions 原始条件列表
    * @return 索引选择结果
    */
    IndexSelectionResult evaluateIndexMatch(const IndexMeta& index, 
                                        const ConditionMapping& condition_mapping,
                                        const std::vector<Condition>& conditions) {
        IndexSelectionResult result;
        
        // 计算匹配的前缀长度和等号条件数量
        for (const auto& col_meta : index.cols) {
            if (!condition_mapping.hasColumn(col_meta.name)) break; // 前缀匹配中断
            
            size_t condition_idx = condition_mapping.column_to_index.at(col_meta.name);
            if (conditions[condition_idx].op == OP_EQ) {
                result.equal_count++;
            }
            result.matched_length++;
        }
        
        // 只有匹配长度大于0才认为是有效匹配
        if (result.matched_length > 0) {
            result.found = true;
            // 存储完整的索引列名
            result.index_col_names.reserve(index.cols.size());
            for (const auto& col_meta : index.cols) {
                result.index_col_names.push_back(col_meta.name);
            }
        }
        
        return result;
    }

    /**
    * @brief 选择最优索引
    * @param current_best 当前最优结果
    * @param candidate 候选结果
    * @param total_conditions 总条件数
    * @return 是否应该更新为候选结果
    */
    bool shouldUpdateBestIndex(const IndexSelectionResult& current_best,
                            const IndexSelectionResult& candidate,
                            size_t total_conditions) {
        if (!candidate.found) return false;
        
        if (!current_best.found) return true;
        
        // 优先级1: 如果候选索引匹配长度更大且小于总条件数，选择它
        if (candidate.matched_length > current_best.matched_length && 
            candidate.matched_length < total_conditions) {
            return true;
        }
        
        // 优先级2: 如果候选索引匹配长度等于总条件数
        if (candidate.matched_length == total_conditions) {
            // 如果当前最优不是完全匹配，选择候选
            if (current_best.matched_length != total_conditions) {
                return true;
            }
            // 如果都是完全匹配，选择等号条件更多的
            if (candidate.equal_count > current_best.equal_count) {
                return true;
            }
        }
        
        return false;
    }

    /**
    * @brief 重新排列条件以匹配索引顺序
    * @param conditions 原始条件列表（会被修改）
    * @param condition_mapping 条件映射
    * @param selected_index_cols 选中的索引列名
    */
    void reorderConditionsForIndex(std::vector<Condition>& conditions,
                                const ConditionMapping& condition_mapping,
                                const std::vector<std::string>& selected_index_cols) {
        std::vector<Condition> reordered_conditions;
        reordered_conditions.reserve(conditions.size());
        
        // 首先添加按索引顺序的条件
        for (const auto& index_col : selected_index_cols) {
            if (condition_mapping.hasColumn(index_col)) {
                size_t condition_idx = condition_mapping.column_to_index.at(index_col);
                reordered_conditions.push_back(std::move(conditions[condition_idx]));
            }
        }
        
        // 添加不在索引中的剩余条件
        for (const auto& col_name : condition_mapping.available_columns) {
            bool found_in_index = std::find(selected_index_cols.begin(), 
                                        selected_index_cols.end(), 
                                        col_name) != selected_index_cols.end();
            if (!found_in_index) {
                size_t condition_idx = condition_mapping.column_to_index.at(col_name);
                reordered_conditions.push_back(std::move(conditions[condition_idx]));
            }
        }
        
        // 添加重复条件
        for (const auto& [col_name, condition_idx] : condition_mapping.duplicate_conditions) {
            std::ignore = col_name;
            reordered_conditions.push_back(std::move(conditions[condition_idx]));
        }
        
        conditions = std::move(reordered_conditions);
    }


    // // 目前的索引匹配规则为：完全匹配索引字段，且全部为单点查询，不会自动调整where条件的顺序
    // bool Planner::get_index_cols(std::string tab_name, std::vector<Condition> curr_conds, std::vector<std::string>& index_col_names) {
    //     index_col_names.clear();
    //     for(auto& cond: curr_conds) {
    //         if(cond.is_rhs_val && cond.op == OP_EQ && cond.lhs_col.tab_name.compare(tab_name) == 0)
    //             index_col_names.push_back(cond.lhs_col.col_name);
    //     }
    //     TabMeta& tab = sm_manager_->db_.get_table(tab_name);
    //     if(tab.is_index(index_col_names)) return true;
    //     return false;
    // }

    /**
    * @brief 获取表的最优索引列名（重构版本）
    * @param tab_name 表名
    * @param curr_conds 当前条件（会被重新排序）
    * @param index_col_names 输出参数：选中的索引列名
    * @return 是否找到可用索引
    */
    bool Planner::get_index_cols(std::string tab_name, 
                                std::vector<Condition>& curr_conds,
                                std::vector<std::string>& index_col_names) {
        
        // 获取表元数据
        TabMeta& tab = sm_manager_->db_.get_table(tab_name);
        
        // 前置检查
        if (curr_conds.empty() || tab.indexes.empty()) {
            return false;
        }
        
        // 构建条件映射
        ConditionMapping condition_mapping(curr_conds);
        
        // 索引选择
        IndexSelectionResult best_result;
        
        // 遍历所有可用索引，选择最优的
        for (const auto& index_meta : tab.indexes) {
            IndexSelectionResult candidate = evaluateIndexMatch(index_meta, condition_mapping, curr_conds);
            
            if (shouldUpdateBestIndex(best_result, candidate, curr_conds.size())) {
                best_result = std::move(candidate);
            }
        }
        
        // 检查是否找到合适的索引
        if (!best_result.found) {
            return false;
        }
        
        // 输出选中的索引列名
        index_col_names = std::move(best_result.index_col_names);
        
        // 重新排列条件以匹配索引顺序
        reorderConditionsForIndex(curr_conds, condition_mapping, index_col_names);
        
        return true;
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
        //谓词下推
        predicate_pushdown(query, context);
        
        //投影下推
        projection_pushdown(query, context);
        return query;
    }

    void Planner::predicate_pushdown(std::shared_ptr<Query> query, Context *context)
    {
        // 常量传播优化
        std::map<TabCol, Value> map;
        //找出"列=常量"的等值条件
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
                        // 创建新条件:,列1=常量
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
                        // 创建新条件, 列2=常量
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

        std::vector<Condition> join_conds;//连接条件
        
        for (const auto& cond : query->conds) {
            if (!cond.is_rhs_val) {
                // 检查是否是连接条件
                if (cond.lhs_col.tab_name != cond.rhs_col.tab_name) {
                    join_conds.push_back(cond);
                    continue;
                }
            }
        }
        
        // 更新query中的条件
        //query->conds.insert(query->conds.end(), join_conds.begin(), join_conds.end());
    }

    void Planner::projection_pushdown(std::shared_ptr<Query> query, Context *context)
    {
        // 检查是否是SELECT* 查询
        bool is_select_star = false;
        if (auto select_stmt = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse)) {
            if (select_stmt->cols.empty() || 
                (select_stmt->cols.size() == 1 && select_stmt->cols[0]->col->col_name == "*")) {
                is_select_star = true;
                std::cout << "DEBUG: 检测到 SELECT *，跳过投影下推分析" << std::endl;
            }
        }
        // 为每个表分开跟踪过滤列和输出列
        std::map<std::string, std::set<std::string>> filter_cols_by_table;
        std::map<std::string, std::set<std::string>> output_cols_by_table;
        std::map<std::string, std::set<std::string>> join_cols_by_table;
        // 为每个表初始化需要的列集合
        //std::map<std::string, std::set<std::string>> required_cols_by_table;
        
         // 如果不是SELECT*，才收集SELECT子句中的列
    if (!is_select_star) {
        // 从SELECT子句收集需要的列 - 这些是输出列
        for (const auto& col : query->cols) {
            if (!col.tab_name.empty() && !col.col_name.empty()) {
                output_cols_by_table[col.tab_name].insert(col.col_name);
            }
        }
    }
    // 处理GROUP BY列 
    if (!query->group_bys.empty()) {
        for (const auto& group_col : query->group_bys) {
            const std::string& table_name = group_col.tab_name;
            const std::string& col_name = group_col.col_name;
            
            // 使用std::find检查表是否存在
            if (std::find(query->tables.begin(), query->tables.end(), table_name) != query->tables.end()) {
                output_cols_by_table[table_name].insert(col_name);
                std::cout << "DEBUG: 添加GROUP BY列到投影: " << table_name << "." << col_name << std::endl;
            }
        }
    }
     // 处理聚合函数中的列
     if (!query->cols.empty() && !query->agg_types.empty()) {
        for (size_t i = 0; i < query->cols.size(); ++i) {
            if (i < query->agg_types.size() && query->agg_types[i] != NO_AGG) {
                const TabCol& col = query->cols[i];
                if (!col.tab_name.empty() && !col.col_name.empty()) {
                    output_cols_by_table[col.tab_name].insert(col.col_name);
                    std::cout << "DEBUG: 添加聚合函数列到投影: " << col.tab_name 
                            << "." << col.col_name << std::endl;
                }
            }
        }
    }
    // 处理HAVING子句中的列
    if (!query->havings.empty()) {
        for (const auto& having_cond : query->havings) {
            // 处理左侧列
            if (!having_cond.lhs_col.tab_name.empty()) {
                output_cols_by_table[having_cond.lhs_col.tab_name].insert(having_cond.lhs_col.col_name);
                std::cout << "DEBUG: 添加HAVING列到投影: " << having_cond.lhs_col.tab_name 
                        << "." << having_cond.lhs_col.col_name << std::endl;
            }
            
            // 处理右侧列(如果不是常量值)
            if (!having_cond.is_rhs_val && !having_cond.rhs_col.tab_name.empty()) {
                output_cols_by_table[having_cond.rhs_col.tab_name].insert(having_cond.rhs_col.col_name);
                std::cout << "DEBUG: 添加HAVING列到投影: " << having_cond.rhs_col.tab_name 
                        << "." << having_cond.rhs_col.col_name << std::endl;
            }
        }
    }
    // 从WHERE条件中收集需要的列 - 这些是过滤列
    for (const auto& cond : query->conds) {
        // 处理左侧列
        if (!cond.lhs_col.tab_name.empty()) {
            filter_cols_by_table[cond.lhs_col.tab_name].insert(cond.lhs_col.col_name);
        }
        
        // 处理右侧列
        if (!cond.is_rhs_val && !cond.rhs_col.tab_name.empty()) {
            filter_cols_by_table[cond.rhs_col.tab_name].insert(cond.rhs_col.col_name);
        }
    }
    
    // 从JOIN条件中收集需要的列 - 这些是连接列
    for (const auto& join_conds : query->join_conds) {
        for (const auto& cond : join_conds) {
            // 处理左侧列
            std::string lhs_tab_name = cond.lhs_col.tab_name;
            for (const auto& [table, alias] : query->tab_alias_map) {
                if (alias == lhs_tab_name) {
                    lhs_tab_name = table; // 转换回原表名
                    break;
                }
            }
            join_cols_by_table[lhs_tab_name].insert(cond.lhs_col.col_name);
            
            // 处理右侧列
            if (!cond.is_rhs_val) {
                std::string rhs_tab_name = cond.rhs_col.tab_name;
                for (const auto& [table, alias] : query->tab_alias_map) {
                    if (alias == rhs_tab_name) {
                        rhs_tab_name = table; // 转换回原表名
                        break;
                    }
                }
                join_cols_by_table[rhs_tab_name].insert(cond.rhs_col.col_name);
            }
        }
    }
    
    // 合并所有需要的列
    std::map<std::string, std::set<std::string>> required_cols_by_table;
    
    // 如果是SELECT*，将所有表标记为需要所有列
    if (is_select_star) {
        for (const auto& table : query->tables) {
            required_cols_by_table[table].clear(); // 需要所有列
        }
    } else {
        // 合并输出列、连接列和过滤列
        for (const auto& table : query->tables) {
            // 首先加入输出列
            if (output_cols_by_table.find(table) != output_cols_by_table.end()) {
                required_cols_by_table[table].insert(
                    output_cols_by_table[table].begin(), output_cols_by_table[table].end());
            }
            
            // 加入连接列
            if (join_cols_by_table.find(table) != join_cols_by_table.end()) {
                required_cols_by_table[table].insert(
                    join_cols_by_table[table].begin(), join_cols_by_table[table].end());
            }
            
            // 过滤列单独存储，不合并到required_cols
            if (filter_cols_by_table.find(table) != filter_cols_by_table.end()) {
                for (const auto& col : filter_cols_by_table[table]) {
                    // 检查此列是否用于连接不同表（WHERE子句中的隐式连接）
                    bool is_join_column = false;
                    for (const auto& cond : query->conds) {
                        if (!cond.is_rhs_val && 
                            cond.lhs_col.tab_name != cond.rhs_col.tab_name) {
                            // 这是一个连接不同表的条件
                            if ((cond.lhs_col.tab_name == table && 
                                 cond.lhs_col.col_name == col) ||
                                (cond.rhs_col.tab_name == table && 
                                 cond.rhs_col.col_name == col)) {
                                is_join_column = true;
                                break;
                            }
                        }
                    }
                    
                    if (is_join_column) {
                        // 这是连接列，必须保留
                        required_cols_by_table[table].insert(col);
                    } else {
                        // 非连接列，可以单独存储
                        query->table_filter_cols[table].insert(col);
                    }
                }
            }
            
            // 更准确的判断逻辑
          if (required_cols_by_table[table].size() > 0) {
    // 获取表的元数据
    TabMeta &tab = sm_manager_->db_.get_table(table);
    
    // 收集表的所有列名
    std::set<std::string> all_col_names;
    for (const auto& col : tab.cols) {
        all_col_names.insert(col.name);
    }
                
                // 只有当所有列都被需要，且需要的列数与表的列数完全匹配时，才标记为"需要所有列"
                if (required_cols_by_table[table].size() == all_col_names.size()) {
                    bool exact_match = true;
                    for (const auto& col_name : required_cols_by_table[table]) {
                        if (all_col_names.find(col_name) == all_col_names.end()) {
                            exact_match = false;
                            break;
                        }
                    }
                    if (exact_match) {
                        required_cols_by_table[table].clear(); // 标记为需要所有列
                        std::cout << "DEBUG: 表 " << table << " 确实需要所有列" << std::endl;
                    }
                }
            }
        }
    }
    
    // 输出调试信息
    for (const auto& [table, cols] : required_cols_by_table) {
        std::cout << "DEBUG: 表 " << table << " 最终需要的列: ";
        if (cols.empty()) {
            std::cout << "所有列";
        } else {
            for (const auto& col : cols) std::cout << col << " ";
        }
        std::cout << std::endl;
    }
    
    // 保存到Query对象中供后续使用
    query->table_required_cols = required_cols_by_table;

    }

    size_t Planner::get_table_cardinality(const std::string& tab_name) {
        // 直接使用表的元组计数
        return sm_manager_->db_.get_table(tab_name).tuple_count;
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
        CompOp op) {
        // 获取两个表的大小
        size_t lhs_size = get_table_cardinality(lhs_tab);
        size_t rhs_size = get_table_cardinality(rhs_tab);
        
        // // 估计不同值的数量 (NDV)
        // size_t lhs_ndv = estimate_ndv(lhs_tab, lhs_col);
        // size_t rhs_ndv = estimate_ndv(rhs_tab, rhs_col);
        
        // // 对于等值连接，选择率 = 1 / max(NDV)
        // if (op == OP_EQ) {
        //     return 1.0 / std::max<double>(lhs_ndv, 1.0);
        // } 
        // // 对于非等值连接，使用更保守的估计
        // else {
        //     return 0.3;  // 可以基于具体操作符进行更精细的估计
        // }
        if (op == OP_EQ) {
            size_t lhs_ndv = estimate_ndv(lhs_tab, lhs_col);
            size_t rhs_ndv = estimate_ndv(rhs_tab, rhs_col);
            return 1.0 / std::max<size_t>(lhs_ndv, rhs_ndv);
        } 
        else {
            // 非等值连接，使用更精确的估计
            switch(op) {
                case OP_LT:
                case OP_GT:
                    return 0.3;  // 经验值
                case OP_LE:
                case OP_GE:
                    return 0.33; // 稍高于>和<
                case OP_NE:
                    return 0.9;  // 大多数行可能会匹配
                default:
                    return 0.5;  // 默认估计
            }
        }
    }

    // 估计列的不同值数量
    size_t Planner::estimate_ndv(const std::string& tab_name, const std::string& col_name) {
        TabMeta& tab = sm_manager_->db_.get_table(tab_name);
        size_t table_size = tab.tuple_count;
        return static_cast<size_t>(sqrt(table_size * log(table_size)));
    }

    std::shared_ptr<Plan> Planner::physical_optimization(std::shared_ptr<Query> query, Context *context)
    {
        std::cout << "DEBUG: physical_optimization 开始" << std::endl;
        std::shared_ptr<Plan> plan = make_one_rel(query);
        std::cout << "DEBUG: make_one_rel 完成" << std::endl;
        // 其他物理优化

        // 处理orderby
        // plan = generate_sort_plan(query, std::move(plan)); 
        std::cout << "DEBUG: generate_sort_plan 完成" << std::endl;
        return plan;
    }


    /**
    * @brief 负责生成物理执行计划中的表访问和表连接部分。
    * "one_rel"中的"rel"是"relation"(关系)的缩写，表示该函数将多个关系（表）整合成一个完整的执行计划。
    */
    std::shared_ptr<Plan> Planner::make_one_rel(std::shared_ptr<Query> query)
    {
        std::cout << "DEBUG: make_one_rel 开始" << std::endl;
        auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
        std::vector<std::string> tables = query->tables;
        std::cout << "DEBUG: 处理的表数量: " << tables.size() << std::endl;
        
        bool is_select_star = false;
        if (auto select_stmt = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse)) {
            if (select_stmt->cols.empty() || 
                (select_stmt->cols.size() == 1 && select_stmt->cols[0]->col->col_name == "*")) {
                is_select_star = true;
            }
        }
        // 检查是否存在半连接条件
        bool has_semi_join = false;
        // 先检查普通条件
        for (const auto& cond : query->conds) {
            if (cond.is_semi_join) {
                has_semi_join = true;
                std::cout << "DEBUG: 检测到半连接条件，禁用表顺序优化" << std::endl;
                break;
            }
        }
        // 再检查JOIN条件
        if (!has_semi_join) {
            for (const auto& join_conds : query->join_conds) {
                for (const auto& cond : join_conds) {
                    if (cond.is_semi_join) {
                        has_semi_join = true;
                        std::cout << "DEBUG: 检测到SEMI JOIN ON条件，禁用表顺序优化" << std::endl;
                        break;
                    }
                }
                if (has_semi_join) break;
            }
        }

        // 为每个表创建扫描执行器
        std::vector<std::shared_ptr<Plan>> table_scan_executors(tables.size());
        std::vector<size_t> table_cardinalities(tables.size());
        
        // 创建扫描计划
        for (size_t i = 0; i < tables.size(); i++) {
            //谓词下推
            auto curr_conds = pop_conds(query->conds, tables[i]);
            std::cout <<"DEBUG: 谓词下推完成"<< std::endl;

            // 检查是否可以使用索引
            std::vector<std::string> index_col_names;
            bool index_exist = get_index_cols(tables[i], curr_conds, index_col_names);
            
            // 创建基本的不带条件的扫描计划
            std::shared_ptr<Plan> scan_plan;
            if (!index_exist) {
                index_col_names.clear();
                scan_plan = std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, tables[i], 
                                                    curr_conds, index_col_names);
            } else {
                scan_plan = std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, tables[i], 
                    curr_conds, index_col_names);
            }
            std::cout <<"DEBUG: 创建基本扫描计划完成"<< std::endl;
            
            //将scan_plan保存到table_scan_executors数组中
            table_scan_executors[i] = scan_plan;

            // 获取表的行数
            std::cout << "DEBUG: 获取表 " << tables[i] << " 的基数" << std::endl;
            table_cardinalities[i] = get_table_cardinality(tables[i]);
            std::cout <<"DEBUG: 获取表基数完成"<< std::endl;

            // 应用投影下推：只在以下情况应用：
            // 不是SELECT* 
            // 不需要表的所有列


            // 修改扫描节点的投影应用逻辑:
            if (!is_select_star) {
                for (size_t j = 0; j < tables.size(); j++) {
                    // 检查是否有特定的列需求
                    if (query->table_required_cols.find(tables[j]) != query->table_required_cols.end() &&
                        !query->table_required_cols[tables[j]].empty()) {
                        
                        auto scan = std::dynamic_pointer_cast<ScanPlan>(table_scan_executors[j]);
                        if (scan) {
                            // 先应用过滤节点
                            // 然后添加投影，只包含永久需要的列
                            std::vector<TabCol> proj_cols;
                            for (const auto& col_name : query->table_required_cols[tables[j]]) {
                                proj_cols.push_back({.tab_name = tables[j], .col_name = col_name});
                            }
                            
                            if (!proj_cols.empty()) {
                                // 创建只有永久列的投影计划
                                table_scan_executors[j] = std::make_shared<ProjectionPlan>(
                                    T_Projection, table_scan_executors[j], proj_cols, 
                                    query->alias, false);
                            }
                        }
                    }
                }
            }
        }
        std::cout <<"DEBUG: 创建扫描计划完成"<< std::endl;
        // 只有一个表，不需要连接
        if (tables.size() == 1) {
            return table_scan_executors[0];
        }
        
        // 连接顺序优化
        // 使用vector跟踪已加入连接计划和未加入的表
        std::vector<bool> used(tables.size(), false);
        std::vector<size_t> join_order;
        
        if (has_semi_join) {
            // 半连接使用原始表顺序，跳过优化
            std::cout << "DEBUG: 半连接使用原始表顺序" << std::endl;
            for (size_t i = 0; i < tables.size(); i++) {
                join_order.push_back(i);
                used[i] = true;
            }
        }
        else{
        // 首先选择基数最小的表
        size_t min_idx = 0;

        for (size_t i = 0; i < tables.size(); i++) {
            // 优先选择基数最小的表
            if ( table_cardinalities[i] < table_cardinalities[min_idx]) {
                min_idx = i;
            }
        }
        
        join_order.push_back(min_idx);
        used[min_idx] = true;
        std::cout <<"DEBUG: 基数最小的表查找完成"<< std::endl;
        // 选择基数第二小的表
        size_t second_min_idx = SIZE_MAX;
        size_t second_min_cardinality = SIZE_MAX;

        for (size_t i = 0; i < tables.size(); i++) {
            if (used[i]) continue;
            
            // 直接比较基数，选择基数第二小的表
            if (table_cardinalities[i] < second_min_cardinality) {
                second_min_cardinality = table_cardinalities[i];
                second_min_idx = i;
            }
        }

        join_order.push_back(second_min_idx);
        used[second_min_idx] = true;
        
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
    }
        // 根据优化后的顺序构建左深树
        std::shared_ptr<Plan> join_plan = table_scan_executors[join_order[0]];
        
        for (size_t i = 1; i < join_order.size(); i++) {
            // 找出连接条件
            std::vector<Condition> join_conds;
            for (auto it = query->conds.begin(); it != query->conds.end();) {
                // 优先处理标记为JOIN条件的条件
                if (it->is_join_cond) {
                    // 检查该JOIN条件是否属于当前JOIN级别
                    std::string lhs_real_table = it->lhs_col.tab_name;
                    std::string rhs_real_table = it->rhs_col.tab_name; 
                    
                    // 转换别名到表名
                    for (const auto& [table, alias] : query->tab_alias_map) {
                        if (alias == lhs_real_table) lhs_real_table = table;
                        if (alias == rhs_real_table) rhs_real_table = table;
                    }           
                    // 当前JOIN条件必须恰好连接当前表和之前某个表
                    bool is_current_join = false;       
                    // 检查是否连接当前表
                    bool connects_current = (lhs_real_table == tables[join_order[i]] ||
                                            rhs_real_table == tables[join_order[i]]);    
                    // 检查是否恰好连接之前的一个表
                    bool connects_one_prev = false;
                    if (connects_current) {
                        std::string other_table;
                        if (lhs_real_table == tables[join_order[i]]) {
                            other_table = rhs_real_table;
                        } else {
                            other_table = lhs_real_table;
                        }
                        
                        // 检查另一个表是否在已连接的表中
                        for (size_t j = 0; j < i; j++) {
                            if (other_table == tables[join_order[j]]) {
                                connects_one_prev = true;
                                break;
                            }
                        }
                    }
                    
                    // 只有同时满足这些条件，才将JOIN条件分配给当前JOIN节点
                    if (connects_current && connects_one_prev) {
                        // 更新条件中的表名为真实表名
                        it->lhs_col.tab_name = lhs_real_table;  
                        if (!it->is_rhs_val) {
                            it->rhs_col.tab_name = rhs_real_table; 
                        }
                        join_conds.push_back(*it);
                        it = query->conds.erase(it);
                    } else {
                        ++it;
                    }
                    continue;
                }
                // 普通条件
                if (it->is_rhs_val) {
                    ++it;
                    continue;
                } 
                std::string lhs_real_table = it->lhs_col.tab_name;
                std::string rhs_real_table = it->rhs_col.tab_name; 
                
                for (const auto& [table, alias] : query->tab_alias_map) {
                    if (alias == lhs_real_table) lhs_real_table = table;
                    if (alias == rhs_real_table) rhs_real_table = table;
                }
                
                // 当前条件必须恰好连接当前表和之前的一个表
                bool connects_current = (lhs_real_table == tables[join_order[i]] ||
                                        rhs_real_table == tables[join_order[i]]);
                                        
                // 检查是否只连接一个之前的表
                bool connects_one_prev = false;
                if (connects_current) {
                    std::string other_table;
                    if (lhs_real_table == tables[join_order[i]]) {
                        other_table = rhs_real_table;
                    } else {
                        other_table = lhs_real_table;
                    }
                    
                    for (size_t j = 0; j < i; j++) {
                        if (other_table == tables[join_order[j]]) {
                            connects_one_prev = true;
                            break;
                        }
                    }
                }
                
                if (connects_current && connects_one_prev) {
                    // 更新条件中的表名为真实表名
                    it->lhs_col.tab_name = lhs_real_table;  
                    if (!it->is_rhs_val) {
                        it->rhs_col.tab_name = rhs_real_table; 
                    }
                    join_conds.push_back(*it);
                    it = query->conds.erase(it);
                } else {
                    ++it;
                }
            }
            
            PlanTag join_type = T_NestLoop; // 默认为嵌套循环JOIN

            // 检查是否标记为SEMI JOIN
            bool has_semi_join = false;
            for (const auto& cond : join_conds) {
                if (cond.is_semi_join) {
                    has_semi_join = true;
                    std::cout << "DEBUG: 检测到SEMI JOIN条件，设置JOIN类型为T_SemiJoin" << std::endl;
                    break;
                }
            }
            if (has_semi_join) {
                join_type = T_SemiJoin;
            }
            join_plan = std::make_shared<JoinPlan>(
                join_type, 
                join_plan,
                table_scan_executors[join_order[i]],
                join_conds
            );
            // 连接后的中间投影下推
    if (!is_select_star && i < join_order.size() - 1) { // 非最后一个连接
        // 收集当前已连接的表
        std::set<std::string> joined_tables;
        for (size_t j = 0; j <= i; j++) {
            joined_tables.insert(tables[join_order[j]]);
        }
        
        // 收集这些表需要保留的列
        std::vector<TabCol> needed_cols;
        
        // 从最终输出列中收集
        for (const auto& tab_name : joined_tables) {
            if (query->table_required_cols.find(tab_name) != query->table_required_cols.end() && 
                !query->table_required_cols[tab_name].empty()) {
                for (const auto& col_name : query->table_required_cols[tab_name]) {
                    needed_cols.push_back({tab_name, col_name});
                }
            }
        }
        
        // 从后续连接条件中收集必要列
        for (size_t j = i + 1; j < join_order.size(); j++) {
            std::string next_table = tables[join_order[j]];
            for (const auto& cond : query->conds) {
                if (!cond.is_rhs_val) {
                    if ((joined_tables.count(cond.lhs_col.tab_name) > 0 && 
                         cond.rhs_col.tab_name == next_table) ||
                        (joined_tables.count(cond.rhs_col.tab_name) > 0 && 
                         cond.lhs_col.tab_name == next_table)) {
                        
                        if (joined_tables.count(cond.lhs_col.tab_name) > 0) {
                            needed_cols.push_back(cond.lhs_col);
                        }
                        if (joined_tables.count(cond.rhs_col.tab_name) > 0) {
                            needed_cols.push_back(cond.rhs_col);
                        }
                    }
                }
            }
        }
        
        // 去重并创建中间投影
       if (!needed_cols.empty()) {
    std::sort(needed_cols.begin(), needed_cols.end(), 
        [](const TabCol& a, const TabCol& b) {
            if (a.tab_name != b.tab_name) return a.tab_name < b.tab_name;
            return a.col_name < b.col_name;
        });
    needed_cols.erase(std::unique(needed_cols.begin(), needed_cols.end(),
        [](const TabCol& a, const TabCol& b) {
            return a.tab_name == b.tab_name && a.col_name == b.col_name;
        }), needed_cols.end());
    
    join_plan = std::make_shared<ProjectionPlan>(
        T_Projection, join_plan, needed_cols, query->alias, false);
}
    }
        }
        std::cout <<"DEBUG: 构建左深树完成"<< std::endl;
        // 处理剩余条件
        for (auto& cond : query->conds) {
            push_conds(&cond, join_plan);
        }
        
        return join_plan;
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
        // TabCol sel_col;
        // for (auto &col : all_cols) {
        //     if(col.name.compare(x->order->cols->col_name) == 0 )
        //     sel_col = {.tab_name = col.tab_name, .col_name = col.name};
        // }
        return std::make_shared<SortPlan>(T_Sort, std::move(plan), query->order_bys, 
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
                for (const auto& cond : join_cond_set) {
                    // 创建带有特殊标记的条件
                    Condition marked_cond = cond;
                    marked_cond.is_join_cond = true; // 标记为JOIN条件
                    query->conds.push_back(marked_cond);
                }
            }
            // 清空join_conds，防止重复处理
            query->join_conds.clear();
        }
        std::cout << "DEBUG: 开始物理优化" << std::endl;

        //物理优化
        auto sel_cols = query->cols;
        std::shared_ptr<Plan> plannerRoot = physical_optimization(query, context);

        // Check if this is an aggregate query
        bool agg = false;
        // Check if there are GROUP BY clauses
        if (!query->group_bys.empty()) agg = true;
        else // Check if there are any aggregate functions
            for (AggType &agg_type : query->agg_types) 
                if (agg_type != NO_AGG) {
                    agg = true; break;
                }
            

        // Generate aggregate plan if needed（在projection之下增加aggregation节点）
        if (agg) 
            plannerRoot = std::make_shared<AggregatePlan>(
                T_Aggregation,
                std::move(plannerRoot),
                std::move(query->cols),
                std::move(query->agg_types),
                std::move(query->group_bys),
                std::move(query->havings)
            );
        // 查询执行树中表示聚合函数的节点为

        plannerRoot = generate_sort_plan(query, std::move(plannerRoot)); // TODO 此处修改是关联着physical_optimization的

        // 添加limit节点
        if(query->limit >= 0) {
            plannerRoot = std::make_shared<LimitPlan>(T_Limit, std::move(plannerRoot), query->limit);
        }

        // TODO order by在前面physical_optimization已经处理过。难道排序是在聚合函数前面吗？
        std::cout << "DEBUG: 创建投影计划" << std::endl;
        //检查是否为SELECT*
        bool is_select_star = false;
        if (auto select_stmt = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse)) {
            // 如果原始cols为空或只有一个列且为*，则标记为SELECT*
            if (select_stmt->cols.empty() || 
                (select_stmt->cols.size() == 1 && select_stmt->cols[0]->col->col_name == "*")) {
                is_select_star = true;
            }
        }
        // 在create ProjectionPlan之前添加检查
        bool has_matching_projection = false;
        if (auto proj = std::dynamic_pointer_cast<ProjectionPlan>(plannerRoot)) {
            // 手动比较两个向量
            if (proj->sel_cols_.size() == sel_cols.size()) {
                bool all_equal = true;
                for (size_t i = 0; i < sel_cols.size(); ++i) {
                    const TabCol& a = proj->sel_cols_[i];
                    const TabCol& b = sel_cols[i];
                    if (a.tab_name != b.tab_name || a.col_name != b.col_name) {
                        all_equal = false;
                        break;
                    }
                }
                has_matching_projection = all_equal;
            }
        }

        // 只有在没有合适的投影节点时才创建新的
        if (!has_matching_projection) {
            plannerRoot = std::make_shared<ProjectionPlan>(
                T_Projection, 
                std::move(plannerRoot), 
                std::move(sel_cols),
                std::move(query->alias),
                is_select_star
            );
        }
        return plannerRoot;
    }


    std::shared_ptr<Plan> Planner::do_planner(std::shared_ptr<Query> query, Context *context)
    {
        if (auto create_checkpoint = std::dynamic_pointer_cast<ast::CreateCheckpoint>(query->parse)) {
            return std::make_shared<CreateCheckpointPlan>();
        }
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

            // 生成select语句的查询执行计划(projection总是在最顶层)
            std::cout << "DEBUG: 生成select语句的查询执行计划" << std::endl;
            std::map<std::string, std::string> saved_alias_map;
            if (is_explain) {
                saved_alias_map = query->tab_alias_map;  // 保存副本
            }
            std::shared_ptr<Plan> projection = generate_select_plan(std::move(query), context); // 生成select语句的查询执行计划
            plannerRoot = std::make_shared<DMLPlan>(T_select, projection, std::string(), std::vector<Value>(),
                                                        std::vector<Condition>(), std::vector<SetClause>());
                                                        // 创建ExplainPlan并设置别名映射

            if (is_explain) {
                std::cout << "DEBUG: 生成ExplainPlan" << std::endl;
                auto explain_plan = std::make_shared<ExplainPlan>(T_Explain, plannerRoot);
                explain_plan->tab_alias_map = saved_alias_map;  // 使用保存的副本
                std::cout << "DEBUG: 别名映射设置完成，大小: " << saved_alias_map.size() << std::endl;
                return explain_plan;
            }
        } else {
            throw InternalError("Unexpected AST root");
        }
        if (is_explain) {
            std::cout << "DEBUG: 生成ExplainPlan" << std::endl;
            auto explain_plan = std::make_shared<ExplainPlan>(T_Explain, plannerRoot);
            if (query) {
                size_t alias_size = query->tab_alias_map.size();      
                if (alias_size > 0) {
                        std::map<std::string, std::string> safe_copy;
                        std::vector<std::string> keys;
                        for (const auto& item : query->tab_alias_map) {
                            keys.push_back(item.first);
                        }//复制
                        for (const auto& key : keys) {
                            safe_copy[key] = query->tab_alias_map[key];
                        }
                        explain_plan->tab_alias_map = safe_copy;
                        std::cout << "DEBUG: 别名映射复制完成" << std::endl;
                }
            } else {
                std::cerr << "ERROR: query对象无效" << std::endl;
            }
            return explain_plan;
        }
        std::cout << "DEBUG: doplanner完成" << std::endl;
        return plannerRoot;
    }
