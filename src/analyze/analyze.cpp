/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "analyze.h"

/**
 * @description: 分析器，进行语义分析和查询重写，需要检查不符合语义规定的部分
 * @param {shared_ptr<ast::TreeNode>} parse parser生成的结果集
 * @return {shared_ptr<Query>} Query 
 */
std::shared_ptr<Query> Analyze::do_analyze(std::shared_ptr<ast::TreeNode> parse)
{
    std::shared_ptr<Query> query = std::make_shared<Query>();//初始化空query
    query->parse = parse;
    if (auto explain_stmt = std::dynamic_pointer_cast<ast::ExplainStmt>(parse)) {
        // 复用 SelectStmt 的处理逻辑
        query->parse = explain_stmt->select;
        // 标记这是 EXPLAIN 查询
        query->is_explain = true; 
        // 处理选择语句内容
        auto x = explain_stmt->select;
        // 处理表名
        //query->tables = std::move(x->tabs);
        analyze_table_refs(x->tabs, query);
        /** 检查表是否存在 */
        for (auto &tab_name : query->tables) {
            if (!sm_manager_->db_.is_table(tab_name)) {
                throw TableNotFoundError(tab_name);
            }
        }
        // 处理target list
        for (auto &sv_sel_col : x->cols) {
            TabCol sel_col = {.tab_name = sv_sel_col->col->tab_name, .col_name = sv_sel_col->col->col_name};
            query->cols.push_back(sel_col);
        }
        
        std::vector<ColMeta> all_cols;
        get_all_cols(query->tables, all_cols);
        if (query->cols.empty()) { 
            // select all columns
            for (auto &col : all_cols) {
                TabCol sel_col = {.tab_name = col.tab_name, .col_name = col.name};
                query->cols.push_back(sel_col);
            }
        } else {
            // infer table name from column name
            for (auto &sel_col : query->cols) {
                sel_col = check_column(all_cols, sel_col, query->tab_alias_map);  // 列元数据校验
            }
        }

        //JOIN处理代码
        if (!x->jointree.empty()) {
            // 处理JOIN条件
            for (const auto& join_expr : x->jointree) {
                // 解析表名，去掉别名部分
                std::string left_table = join_expr->left;
                std::string right_table = join_expr->right;
                // 处理左表可能的别名
                size_t left_space = left_table.find(' ');
                if (left_space != std::string::npos) {
                    left_table = left_table.substr(0, left_space);
                }    
                // 处理右表可能的别名
                size_t right_space = right_table.find(' ');
                if (right_space != std::string::npos) {
                    right_table = right_table.substr(0, right_space);
                }
                // 确保表存在
                if (!sm_manager_->db_.is_table(left_table)) {
                    throw TableNotFoundError(left_table);
                }
                if (!sm_manager_->db_.is_table(right_table)) {
                    throw TableNotFoundError(right_table);
                }
                
                // 处理JOIN条件
                std::vector<Condition> join_conds;
                get_clause(join_expr->conds, join_conds);
                for (auto& cond : join_conds) {
                    cond.is_join_cond = true;  //标记为JOIN
                    if (join_expr->type == SEMI_JOIN) {
                        cond.is_semi_join = true;  //标记为SEMI JOIN
                        std::cout << "DEBUG: 检测到SEMI JOIN条件" << std::endl;
                    }
                }
                query->join_conds.push_back(join_conds);
                // 检查JOIN条件
                check_clause({left_table, right_table}, join_conds,query->tab_alias_map);
            }
        }

        //处理where条件
        get_clause(x->conds, query->conds);
        check_clause(query->tables, query->conds, false);
    }
    else if (auto x = std::dynamic_pointer_cast<ast::SelectStmt>(parse))//select语句
    {
        /* 大致处理顺序： 表名-->聚合函数类型-->where-->group by-->sort/having */
        std::cout << "DEBUG: Processing SelectStmt, explain=" << x->explain << std::endl;
        // 从SelectStmt中传递explain标志到Query对象
        query->is_explain = x->explain;
        // 处理表名
        analyze_table_refs(x->tabs, query);
        /** TODO: 检查表是否存在 */
        for (auto &tab_name : query->tables) {
            if (!sm_manager_->db_.is_table(tab_name)) {
                throw TableNotFoundError(tab_name);
            }
        }

       // TODO orderby不能使用别名

        for (auto &item: x->cols) {
            query->cols.emplace_back(TabCol{item->col->tab_name, item->col->col_name});
            query->agg_types.emplace_back(item->type);
            if (query->agg_types.back() != NO_AGG && item->alias.empty()) {
                switch (query->agg_types.back()) {
                    case AGG_COUNT: 
                        if (item->col->col_name.empty()) query->alias.emplace_back("COUNT(*)");
                        else query->alias.emplace_back("COUNT(" + item->col->col_name + ")");
                        break;
                    case AGG_MAX: 
                        query->alias.emplace_back("MAX(" + item->col->col_name + ")");
                        break;
                    case AGG_MIN: 
                        query->alias.emplace_back("MIN(" + item->col->col_name + ")");
                        break;
                    case AGG_SUM: 
                        query->alias.emplace_back("SUM(" + item->col->col_name + ")");
                        break;
                    case AGG_AVG: 
                        query->alias.emplace_back("AVG(" + item->col->col_name + ")");
                        break;
                    default:
                        throw InternalError("Unexpected aggregation type!");
                }
            } else 
                query->alias.emplace_back(std::move(item->alias));
            
        }

        std::vector<ColMeta> all_cols;
        get_all_cols(query->tables, all_cols);
        if (query->cols.empty()) { // select all columns(select *)
            if (!x->group_bys.empty() || !x->havings.empty()) 
                throw InternalError("Aggregation functions and GROUP BY cannot be used without a target list.");

            for (auto &col : all_cols) {
                TabCol sel_col = {.tab_name = col.tab_name, .col_name = col.name};
                query->cols.push_back(sel_col);
            }
        } else {
            // infer table name from column name
            bool might_have_semi_join = false;
            for (const auto& join_ref : x->jointree) {
                if (join_ref->type == SEMI_JOIN) {
                    might_have_semi_join = true;
                    break;
                }
            }
            // 如果不是半连接，执行正常的列检查
            // 如果可能是半连接，则推迟到JOIN处理后进行
            if (!might_have_semi_join) {
                // infer table name from column name
                for (size_t i = 0; i < query->cols.size(); i++) {
                TabCol &sel_col = query->cols[i];
                if (query->agg_types[i] == AGG_COUNT && 
                    query->cols[i].tab_name.empty() && 
                    query->cols[i].col_name.empty()) 
                    continue; // 特判count(*)
                    sel_col = check_column(all_cols, sel_col, query->tab_alias_map);  // 列元数据校验
                }
            } else {
                std::cout << "DEBUG: 检测到半连接，推迟列检查" << std::endl;
            }
        }

        // 处理group by条件
        for (auto &group_by: x->group_bys) 
            query->group_bys.emplace_back(TabCol{group_by->tab_name, group_by->col_name});
        for (auto &tab_col: query->group_bys) // 校验group by的列
            tab_col = check_column(all_cols, tab_col, query->tab_alias_map);  //! 注意赋值
        
        // 检验group by与target list的列的兼容性
        if(x->group_bys.empty()) { // 没有group by
            if(!x->havings.empty()) // 有having
                throw InternalError("HAVING clause cannot be used without GROUP BY.");

            bool has_agg = query->agg_types.size() > 0 && query->agg_types[0] != NO_AGG;
            for(AggType agg_type: query->agg_types) { // 要么全是聚合函数，要么全不是
                if((agg_type != NO_AGG) != has_agg) 
                    throw InternalError("All columns in target list must either be aggregated or not aggregated.");
            }
        } else { // 有group by
            for (size_t i = 0; i < query->cols.size(); i++) {
                TabCol &col = query->cols[i];
                if (query->agg_types[i] == NO_AGG) { 
                    bool found = false;
                    for (auto &group_by : query->group_bys) 
                        if (col.tab_name == group_by.tab_name && col.col_name == group_by.col_name) {
                            found = true; break;
                        }
                    if (!found) 
                        throw InternalError("Column " + col.col_name + " must be in GROUP BY clause.");
                }
            }
        }

        if(x->has_sort) {
            TabCol tab_col = TabCol{std::move(x->order->cols->tab_name), 
                                    std::move(x->order->cols->col_name)};
            // 使用带别名映射的check_column版本
            tab_col = check_column(all_cols, tab_col, query->tab_alias_map);
            query->order_bys = std::move(tab_col);
        }

        // 处理limit子句
        if(x->limit) {
            if (x->limit->limit_num < 0) {
                throw InternalError("LIMIT clause must be a non-negative integer.");
            }
            query->limit = x->limit->limit_num; // 处理limit
        } else {
            query->limit = -1; // 没有限制
        }

        //JOIN处理代码 // TODO放的位置是否正确
        if (!x->jointree.empty()) {
            // 处理JOIN条件
            for (const auto& join_expr : x->jointree) {
                // 解析表名，去掉别名部分
                std::string left_table = join_expr->left;
                std::string right_table = join_expr->right;
                // 处理左表可能的别名
                size_t left_space = left_table.find(' ');
                if (left_space != std::string::npos) {
                    left_table = left_table.substr(0, left_space);
                }
                
                // 处理右表可能的别名
                size_t right_space = right_table.find(' ');
                if (right_space != std::string::npos) {
                    right_table = right_table.substr(0, right_space);
                }
                // 确保表存在
                if (!sm_manager_->db_.is_table(left_table)) {
                    throw TableNotFoundError(left_table);
                }
                if (!sm_manager_->db_.is_table(right_table)) {
                    throw TableNotFoundError(right_table);
                }
                
                // 处理JOIN条件
                std::vector<Condition> join_conds;
                get_clause(join_expr->conds, join_conds);
                for (auto& cond : join_conds) {
                    cond.is_join_cond = true;  //标记为JOIN
                    if (join_expr->type == SEMI_JOIN) {
                        cond.is_semi_join = true;  //标记为SEMI JOIN
                        std::cout << "DEBUG: 检测到SEMI JOIN条件" << std::endl;}
                }
                query->join_conds.push_back(join_conds);
                // 检查JOIN条件
                check_clause({left_table, right_table}, join_conds,query->tab_alias_map);
            }
            bool has_semi_join = false;
            std::set<std::string> left_tables; // 保存所有半连接的左表

            for (const auto& join_expr : x->jointree) {
                if (join_expr->type == SEMI_JOIN) {
                    has_semi_join = true;
                    
                    // 提取左表名（去掉可能的别名）
                    std::string left_table = join_expr->left;
                    size_t space_pos = left_table.find(' ');
                    if (space_pos != std::string::npos) {
                        left_table = left_table.substr(0, space_pos);
                    }
                    
                    // 将左表添加到集合中
                    left_tables.insert(left_table);
                    std::cout << "DEBUG: 半连接左表: " << left_table << std::endl;
                }
            }

            // 如果存在半连接，筛选列，只保留左表的列
            if (has_semi_join) {
                std::vector<TabCol> filtered_cols;
                std::vector<std::string> invalid_cols; // 收集无效列名
                
                for (const auto& col : query->cols) {
                    TabCol checked_col = check_column(all_cols, 
                                                     col, 
                                                     query->tab_alias_map, 
                                                     has_semi_join, left_tables);
                    std::string real_tab_name = checked_col.tab_name;
                    
                    // 处理可能的表别名
                    auto it = query->tab_alias_map.find(real_tab_name);
                    if (it != query->tab_alias_map.end()) {
                        real_tab_name = it->first;
                    } else {
                        // 反向查找别名
                        for (const auto& [table, alias] : query->tab_alias_map) {
                            if (alias == real_tab_name) {
                                real_tab_name = table;
                                break;
                            }
                        }
                    }
                    
                    // 检查列是否属于左表
                    if (left_tables.find(real_tab_name) != left_tables.end()) {
                        filtered_cols.push_back(checked_col);
                    } else {
                        // 收集来自右表的列名
                        invalid_cols.push_back(checked_col.col_name);
                    }
                }
                
                // 如果有任何来自右表的列，抛出错误
                if (!invalid_cols.empty()) {
                    std::string invalid_cols_str;
                    for (size_t i = 0; i < invalid_cols.size(); ++i) {
                        if (i > 0) invalid_cols_str += ", ";
                        invalid_cols_str += invalid_cols[i];
                    }
                    throw SemiJoinColumnError(invalid_cols_str);
                }
                
                // 更新查询列
                query->cols = filtered_cols;
                std::cout << "DEBUG: 半连接后列过滤：保留 " << query->cols.size() << " 列" << std::endl;
            }
    }

        //处理where条件
        
        get_clause(x->conds, query->conds);
        get_having_clause(x->havings, query->havings);
        // check_clause(query->tables, query->havings, true);
        // check_clause(query->tables, query->conds, false);
        check_clause(query->tables, query->conds, query->tab_alias_map, false);
        check_clause(query->tables, query->havings, query->tab_alias_map, true);
    } else if (auto x = std::dynamic_pointer_cast<ast::UpdateStmt>(parse)) {
        // 处理表名
        query->tables.push_back(x->tab_name);
        
        // 检查表是否存在
        if (!sm_manager_->db_.is_table(x->tab_name)) {
            throw TableNotFoundError(x->tab_name);
        }
        
        // 处理更新的set子句
        TabMeta &tab = sm_manager_->db_.get_table(x->tab_name);
        for (auto &sv_set_clause : x->set_clauses) {
            SetClause set_clause;
            set_clause.lhs.tab_name = x->tab_name;
            set_clause.lhs.col_name = sv_set_clause->col_name;
            
            // 根据表达式类型处理
            if (sv_set_clause->expr_type == ast::SIMPLE_VALUE) {
                // 原来的简单赋值处理
                set_clause.rhs = convert_sv_value(sv_set_clause->val);
            } else {
                // 处理列引用表达式
                set_clause.rhs = convert_sv_value(sv_set_clause->val); 
                
                // 检查引用的列是否存在
                TabCol ref_col = {.tab_name = x->tab_name, .col_name = sv_set_clause->ref_col_name};
                
                // 创建临时的cols变量，因为当前上下文中没有all_cols
                std::vector<ColMeta> cols;
                get_all_cols({x->tab_name}, cols);
                ref_col = check_column(cols, ref_col);
                
                // 检查操作数类型兼容性
                auto target_col = tab.get_col(set_clause.lhs.col_name);
                auto ref_col_meta = tab.get_col(ref_col.col_name);
                if (!value_type_match(target_col->type, ref_col_meta->type) ||
                    !value_type_match(target_col->type, set_clause.rhs.type)) {
                    throw IncompatibleTypeError(coltype2str(target_col->type), 
                                               "Expression operand types");
                }
                
                
                set_clause.is_expr = true;                     // 标记为表达式更新
                set_clause.ref_col = ref_col;                  // 引用列
                set_clause.op_type = sv_set_clause->op;        // 运算符
            }
            
            query->set_clauses.push_back(set_clause);
        }
        
        // 处理where条件
        get_clause(x->conds, query->conds);
        
        check_clause({x->tab_name}, query->conds,query->tab_alias_map);
    } else if (auto x = std::dynamic_pointer_cast<ast::DeleteStmt>(parse)) {
        query->tables.push_back(x->tab_name);
        if (!sm_manager_->db_.is_table(x->tab_name)) {
            throw TableNotFoundError(x->tab_name);
        }
        get_clause(x->conds, query->conds);
        check_clause({x->tab_name}, query->conds,query->tab_alias_map);        
    } else if (auto x = std::dynamic_pointer_cast<ast::InsertStmt>(parse)) {
        // 处理insert 的values值
            query->tables.push_back(x->tab_name);
        // 检查表是否存在
        if (!sm_manager_->db_.is_table(x->tab_name)) {
            throw TableNotFoundError(x->tab_name);
        }
        for (auto &sv_val : x->vals) {
            query->values.push_back(convert_sv_value(sv_val));
        }
    } else {
        // do nothing
    }
    query->parse = std::move(parse);
    return query;
}


TabCol Analyze::check_column(const std::vector<ColMeta> &all_cols, TabCol target,
    const std::map<std::string, std::string> &tab_alias_map,
    bool is_semi_join, const std::set<std::string> &left_tables) {
    // 检查是否是别名，如果是则转换为原表名
    std::string real_tab_name = target.tab_name;
    //反向查找别名对应的真实表名
    for (const auto& [table, alias] : tab_alias_map) {
        if (alias == target.tab_name) {
            real_tab_name = table;
            std::cout << "DEBUG: 将别名 '" << target.tab_name << "' 转换为表名 '" << real_tab_name << "'" << std::endl;
            break;
        }
    }
    // 更新target中的表名为真实表名
    target.tab_name = real_tab_name;
    if (target.tab_name.empty()) {
        // Table name not specified, infer table name from column name
        std::string tab_name;
        std::vector<std::string> matched_tables;
        
        // 收集所有匹配的表
        for (auto &col : all_cols) {
            if (col.name == target.col_name) {
                matched_tables.push_back(col.tab_name);
                if (tab_name.empty()) {
                    tab_name = col.tab_name;
                }
            }
        }
        
        // 如果有多个匹配，检查是否是半连接情况
        if (matched_tables.size() > 1) {
            if (is_semi_join && !left_tables.empty()) {
                for (const auto& tab : matched_tables) {
                    if (left_tables.find(tab) != left_tables.end()) {
                        tab_name = tab;
                        std::cout << "DEBUG: 半连接中消除列歧义，选择左表 " << tab_name 
                                  << " 的列 " << target.col_name << std::endl;
                        target.tab_name = tab_name;
                        return target;
                    }
                }
            }
            
            // 如果不是半连接或找不到匹配的左表列，抛出歧义错误
            throw AmbiguousColumnError(target.col_name);
        }
        
        if (tab_name.empty()) {
            throw ColumnNotFoundError(target.col_name);
        }
        target.tab_name = tab_name;
    } else {
        /** TODO: Make sure target column exists */
        bool found = false;
        for (auto &col : all_cols) {
            std::cout << "DEBUG: 检查列 " << col.tab_name << "." << col.name 
                      << " 是否与目标列 " << target.tab_name << "." << target.col_name 
                      << " 匹配" << std::endl;
            if (col.tab_name == target.tab_name && col.name == target.col_name) {
                found = true;
                break;
            }
        }        
        if (!found) {
            //列在指定的表中不存在，判断是表不存在还是列不存在
            if (sm_manager_->db_.is_table(target.tab_name)) {
                throw ColumnNotFoundError(target.col_name);
            } else {
                std::cout << "DEBUG:checkcolumn找不到表 " << target.tab_name << std::endl;
                throw TableNotFoundError(target.tab_name);
            }
        }
    }
    return target;
}

void Analyze::get_having_clause(
                const std::vector<std::shared_ptr<ast::HavingExpr> > &having_clause,
                std::vector<Condition> &conds) {
    conds.clear();
    for (auto &expr: having_clause) {
        Condition cond;
        // 左操作数检查
        if (expr->lhs->type == NO_AGG) 
            throw InternalError("Having 语句左侧必须是聚合函数！");
        
        cond.agg_type = expr->lhs->type;
        cond.lhs_col = TabCol{.tab_name = expr->lhs->col->tab_name, 
                              .col_name = expr->lhs->col->col_name};
        cond.op = convert_sv_comp_op(expr->op);

        // 右操作数检查
        if (auto rhs_val = std::dynamic_pointer_cast<ast::Value>(expr->rhs)) {
            cond.is_rhs_val = true;
            cond.rhs_val = convert_sv_value(rhs_val);
        } else if (auto rhs_col = std::dynamic_pointer_cast<ast::Col>(expr->rhs)) 
            throw InternalError("Right side of HAVING clause must be a value, not a column.");
        else 
            throw InternalError("Unexpected right side of HAVING clause.");
        
        conds.emplace_back(std::move(cond));
    }
}

void Analyze::get_all_cols(const std::vector<std::string> &tab_names, std::vector<ColMeta> &all_cols) {
    for (auto &sel_tab_name : tab_names) {
        // 这里db_不能写成get_db(), 注意要传指针
        const auto &sel_tab_cols = sm_manager_->db_.get_table(sel_tab_name).cols;
        all_cols.insert(all_cols.end(), sel_tab_cols.begin(), sel_tab_cols.end());
    }
}

/* 构造where条件对应的Condition */
void Analyze::get_clause(const std::vector<std::shared_ptr<ast::BinaryExpr>> &sv_conds, std::vector<Condition> &conds) {
    conds.clear();
    for (auto &expr : sv_conds) {
        Condition cond;
        cond.rhs_val.set_int(0);  
        cond.lhs_col = {.tab_name = expr->lhs->tab_name, .col_name = expr->lhs->col_name};
        cond.op = convert_sv_comp_op(expr->op);
        if (auto rhs_val = std::dynamic_pointer_cast<ast::Value>(expr->rhs)) {
            cond.is_rhs_val = true;
            cond.rhs_val = convert_sv_value(rhs_val);
        } else if (auto rhs_col = std::dynamic_pointer_cast<ast::Col>(expr->rhs)) {
            cond.is_rhs_val = false;
            cond.rhs_col = {.tab_name = rhs_col->tab_name, .col_name = rhs_col->col_name};
        }
        conds.push_back(cond);
    }
}

/* 检查条件语句的规范性 */
void Analyze::check_clause(const std::vector<std::string> &tab_names, 
                            std::vector<Condition> &conds,
                            bool check_having) {
    // auto all_cols = get_all_cols(tab_names);
    std::vector<ColMeta> all_cols;
    get_all_cols(tab_names, all_cols);
    // Get raw values in where clause
    for (auto &cond : conds) {        
        // 处理条件语句左侧列
        if(!check_having && cond.agg_type != NO_AGG) 
            throw InternalError("Where clause left side mustn't be an aggregation function.");
        if (cond.agg_type == AGG_COUNT && cond.lhs_col.tab_name.empty() && cond.lhs_col.col_name.empty()) {
            cond.rhs_val.init_raw(sizeof(int));
            continue;
        } 
        cond.lhs_col = check_column(all_cols, cond.lhs_col);// Infer table name from column name
        TabMeta &lhs_tab = sm_manager_->db_.get_table(cond.lhs_col.tab_name);
        auto lhs_col = lhs_tab.get_col(cond.lhs_col.col_name);
        ColType lhs_type; 
        if(cond.agg_type == AGG_COUNT) lhs_type = TYPE_INT; // count(*)的类型是int
        else if (cond.agg_type == AGG_AVG) lhs_type = TYPE_FLOAT; // avg的类型是float
        else lhs_type = lhs_col->type; // 获取列的类型

        // 处理条件语句右侧列
        ColType rhs_type;
        if (cond.is_rhs_val) {
            if(cond.agg_type == AGG_COUNT)     cond.rhs_val.init_raw(sizeof(int));
            else if (cond.agg_type == AGG_AVG) cond.rhs_val.init_raw(sizeof(float));
            else cond.rhs_val.init_raw(lhs_col->len);
            rhs_type = cond.rhs_val.type;
        } else {
            cond.rhs_col = check_column(all_cols, cond.rhs_col);
            TabMeta &rhs_tab = sm_manager_->db_.get_table(cond.rhs_col.tab_name);
            auto rhs_col = rhs_tab.get_col(cond.rhs_col.col_name);
            rhs_type = rhs_col->type;
        }

        if (!value_type_match(lhs_type, rhs_type) &&
            !(lhs_type == TYPE_DATE && rhs_type == TYPE_STRING)) 
            throw IncompatibleTypeError(coltype2str(lhs_type), coltype2str(rhs_type));
    }
}

// void Analyze::check_clause(const std::vector<std::string> &tab_names, 
//     std::vector<Condition> &conds,
//     const std::map<std::string, std::string> &tab_alias_map = {},
//     bool check_having) {
//     // auto all_cols = get_all_cols(tab_names);
//     //check_clause(tab_names, conds, check_having);
//     std::vector<ColMeta> all_cols;
//     get_all_cols(tab_names, all_cols);
//     // Get raw values in where clause
//     for (auto &cond : conds) {
//         // Infer table name from column name
//         cond.lhs_col = check_column(all_cols, cond.lhs_col, tab_alias_map);
//         if (!cond.is_rhs_val) {
//             cond.rhs_col = check_column(all_cols, cond.rhs_col, tab_alias_map);
//         }
//         TabMeta &lhs_tab = sm_manager_->db_.get_table(cond.lhs_col.tab_name);
//         auto lhs_col = lhs_tab.get_col(cond.lhs_col.col_name);
//         ColType lhs_type = lhs_col->type;
//         ColType rhs_type;
//         if (cond.is_rhs_val) {
//             cond.rhs_val.init_raw(lhs_col->len);
//             rhs_type = cond.rhs_val.type;
//         } else {
//             TabMeta &rhs_tab = sm_manager_->db_.get_table(cond.rhs_col.tab_name);
//             auto rhs_col = rhs_tab.get_col(cond.rhs_col.col_name);
//             rhs_type = rhs_col->type;
//         }
//         if (!value_type_match(lhs_type, rhs_type)) 
//             throw IncompatibleTypeError(coltype2str(lhs_type), coltype2str(rhs_type));
        
//     }
// }
void Analyze::check_clause(const std::vector<std::string> &tab_names, 
    std::vector<Condition> &conds,
    const std::map<std::string, std::string> &tab_alias_map,
    bool check_having) {
    // 先创建条件的副本，恢复原始表名
    std::vector<Condition> conds_copy = conds;
    // 临时替换条件中的别名为实际表名
    for (auto &cond : conds) {
        // 处理表别名 
        for (const auto& [table, alias] : tab_alias_map) {
            if (cond.lhs_col.tab_name == alias) {
                std::cout << "DEBUG: 临时将别名 '" << alias << "' 转换为表名 '" << table << "'" << std::endl;
                cond.lhs_col.tab_name = table;
            }
            // 处理右侧列的别名
            if (!cond.is_rhs_val && cond.rhs_col.tab_name == alias) {
                cond.rhs_col.tab_name = table;
            }
        }
    }
    // 调用第一个check_clause进行基本检查和HAVING检查
    check_clause(tab_names, conds, check_having);
    // 恢复原始条
    conds = conds_copy;
    // 用别名映射正确处理条件
    std::vector<ColMeta> all_cols;
    get_all_cols(tab_names, all_cols);
    
    for (auto &cond : conds) {
        if (cond.agg_type == AGG_COUNT && cond.lhs_col.tab_name.empty() && cond.lhs_col.col_name.empty())  continue; // count(*)
        // 使用带别名参数的check_column
        cond.lhs_col = check_column(all_cols, cond.lhs_col, tab_alias_map);
        if (!cond.is_rhs_val) {
            cond.rhs_col = check_column(all_cols, cond.rhs_col, tab_alias_map);
        }
        
        //! 冗余
        // TabMeta &lhs_tab = sm_manager_->db_.get_table(cond.lhs_col.tab_name);
        // auto lhs_col = lhs_tab.get_col(cond.lhs_col.col_name);
        // ColType lhs_type = lhs_col->type;
        // ColType rhs_type;
        
        // if (cond.is_rhs_val) {
        //     cond.rhs_val.init_raw(lhs_col->len);
        //     rhs_type = cond.rhs_val.type;
        // } else {
        //     TabMeta &rhs_tab = sm_manager_->db_.get_table(cond.rhs_col.tab_name);
        //     auto rhs_col = rhs_tab.get_col(cond.rhs_col.col_name);
        //     rhs_type = rhs_col->type;
        // }
    }
}

// 判断两个类型是否可以比较
bool Analyze::value_type_match(ColType type1, ColType type2) {
    if (type1 == type2) return true;
    if (type1 == ColType::TYPE_INT && type2 == ColType::TYPE_FLOAT) return true;
    if (type1 == ColType::TYPE_FLOAT && type2 == ColType::TYPE_INT) return true;
    return false;
}


Value Analyze::convert_sv_value(const std::shared_ptr<ast::Value> &sv_val) {
    Value val;
    if (auto int_lit = std::dynamic_pointer_cast<ast::IntLit>(sv_val)) {
        val.set_int(int_lit->val);
    } else if (auto float_lit = std::dynamic_pointer_cast<ast::FloatLit>(sv_val)) {
        val.set_float(float_lit->val);
    } else if (auto str_lit = std::dynamic_pointer_cast<ast::StringLit>(sv_val)) {
        val.set_str(str_lit->val);
    } else if (auto date_lit = std::dynamic_pointer_cast<ast::DateLit>(sv_val)) {
        val.set_date(date_lit->val);
    } else {
        throw InternalError("Unexpected sv value type");
    }
    return val;
}

CompOp Analyze::convert_sv_comp_op(ast::SvCompOp op) {
    std::map<ast::SvCompOp, CompOp> m = {
        {ast::SV_OP_EQ, OP_EQ}, {ast::SV_OP_NE, OP_NE}, {ast::SV_OP_LT, OP_LT},
        {ast::SV_OP_GT, OP_GT}, {ast::SV_OP_LE, OP_LE}, {ast::SV_OP_GE, OP_GE},
    };
    return m.at(op);
}

void Analyze::analyze_table_refs(const std::vector<std::string> &tab_refs, std::shared_ptr<Query> query) {
    std::cout << "DEBUG: 原始表引用: ";
    for (const auto& ref : tab_refs) {
        std::cout << "'" << ref << "' ";
    }
    std::cout << std::endl;
    for (auto &tab_ref : tab_refs) {
        std::string tab_name = tab_ref;
        std::string alias;
        // 检查是否包含空格
        size_t space_pos = tab_name.find(' ');
        if (space_pos != std::string::npos) {
            std::cout << "DEBUG: 检测到表名 '" << tab_name << "' 包含空格，可能有别名" << std::endl;
            alias = tab_name.substr(space_pos + 1);
            tab_name = tab_name.substr(0, space_pos);  
            // 存储别名映射
            query->tab_alias_map[tab_name] = alias;
        }  
        std::cout << "DEBUG: 处理表名 '" << tab_name << "'，别名 '" << alias << "'" << std::endl;
        // 将表名添加到查询中
        query->tables.push_back(tab_name);
    }
}