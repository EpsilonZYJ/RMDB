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
    if (auto explain_stmt = std::dynamic_pointer_cast<ast::ExplainStmt>(parse)) {
        // 复用 SelectStmt 的处理逻辑
        query->parse = explain_stmt->select;
        // 标记这是 EXPLAIN 查询
        query->is_explain = true;
        
        // 处理选择语句内容（与 SelectStmt 相同的逻辑）
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
            TabCol sel_col = {.tab_name = sv_sel_col->tab_name, .col_name = sv_sel_col->col_name};
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
                query->join_conds.push_back(join_conds);
                
                // 检查JOIN条件
                check_clause({left_table, right_table}, join_conds,query->tab_alias_map);
            }
        }

        //处理where条件
        get_clause(x->conds, query->conds);
        check_clause(query->tables, query->conds,query->tab_alias_map);
    }
    else if (auto x = std::dynamic_pointer_cast<ast::SelectStmt>(parse))//select语句
    {
        // 处理表名
        analyze_table_refs(x->tabs, query);
        /** TODO: 检查表是否存在 */
        for (auto &tab_name : query->tables) {
            if (!sm_manager_->db_.is_table(tab_name)) {
                throw TableNotFoundError(tab_name);
            }
        }
        // 处理target list，再target list中添加上表名，例如 a.id
        for (auto &sv_sel_col : x->cols) {
            TabCol sel_col = {.tab_name = sv_sel_col->tab_name, .col_name = sv_sel_col->col_name};
            query->cols.push_back(sel_col);
        }
        
        std::vector<ColMeta> all_cols;
        get_all_cols(query->tables, all_cols);
        if (query->cols.empty()) {
            // select all columns
            for (auto &col : all_cols) {
                TabCol sel_col = {.tab_name = col.tab_name, .col_name = col.name};
                query->cols.push_back(sel_col);
            }//slelect *
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
                query->join_conds.push_back(join_conds);
                
                // 检查JOIN条件
                check_clause({left_table, right_table}, join_conds,query->tab_alias_map);
            }
        }

        //处理where条件
        get_clause(x->conds, query->conds);
        check_clause(query->tables, query->conds,query->tab_alias_map);
        query->is_explain = x->explain;
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
            set_clause.rhs = convert_sv_value(sv_set_clause->val);
            
            // 添加初始化raw字段的代码
            auto col = tab.get_col(set_clause.lhs.col_name);
            set_clause.rhs.init_raw(col->len);
            
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
    const std::map<std::string, std::string> &tab_alias_map = {}) {
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
        for (auto &col : all_cols) {
            if (col.name == target.col_name) {
                if (!tab_name.empty()) {
                    throw AmbiguousColumnError(target.col_name);
                }//列名歧义，已经找到了匹配的表，又出现了另一个，比如Stu.id和Course.id同时出现
                tab_name = col.tab_name;
            }
        }
        if (tab_name.empty()) {
            throw ColumnNotFoundError(target.col_name);
        }//没有找到匹配的列名
        target.tab_name = tab_name;
    } else {
        /** TODO: Make sure target column exists */
        bool found = false;
        for (auto &col : all_cols) {
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
                throw TableNotFoundError(target.tab_name);
            }
        }
    }
    return target;
}

void Analyze::get_all_cols(const std::vector<std::string> &tab_names, std::vector<ColMeta> &all_cols) {
    for (auto &sel_tab_name : tab_names) {
        // 这里db_不能写成get_db(), 注意要传指针
        const auto &sel_tab_cols = sm_manager_->db_.get_table(sel_tab_name).cols;
        all_cols.insert(all_cols.end(), sel_tab_cols.begin(), sel_tab_cols.end());
    }
}

void Analyze::get_clause(const std::vector<std::shared_ptr<ast::BinaryExpr>> &sv_conds, std::vector<Condition> &conds) {
    conds.clear();
    for (auto &expr : sv_conds) {
        Condition cond;
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

// 判断两个类型是否可以比较
bool Analyze::value_type_match(ColType type1, ColType type2) {
    if (type1 == type2) return true;
    if (type1 == ColType::TYPE_INT && type2 == ColType::TYPE_FLOAT) return true;
    if (type1 == ColType::TYPE_FLOAT && type2 == ColType::TYPE_INT) return true;
    return false;
}

void Analyze::check_clause(const std::vector<std::string> &tab_names, std::vector<Condition> &conds,const std::map<std::string, std::string> &tab_alias_map = {}) {
    // auto all_cols = get_all_cols(tab_names);
    std::vector<ColMeta> all_cols;
    get_all_cols(tab_names, all_cols);
    // Get raw values in where clause
    for (auto &cond : conds) {
        // Infer table name from column name
        cond.lhs_col = check_column(all_cols, cond.lhs_col, tab_alias_map);
        if (!cond.is_rhs_val) {
            cond.rhs_col = check_column(all_cols, cond.rhs_col, tab_alias_map);
        }
        TabMeta &lhs_tab = sm_manager_->db_.get_table(cond.lhs_col.tab_name);
        auto lhs_col = lhs_tab.get_col(cond.lhs_col.col_name);
        ColType lhs_type = lhs_col->type;
        ColType rhs_type;
        if (cond.is_rhs_val) {
            cond.rhs_val.init_raw(lhs_col->len);
            rhs_type = cond.rhs_val.type;
        } else {
            TabMeta &rhs_tab = sm_manager_->db_.get_table(cond.rhs_col.tab_name);
            auto rhs_col = rhs_tab.get_col(cond.rhs_col.col_name);
            rhs_type = rhs_col->type;
        }
        if (!value_type_match(lhs_type, rhs_type)) 
            throw IncompatibleTypeError(coltype2str(lhs_type), coltype2str(rhs_type));
    }
}


Value Analyze::convert_sv_value(const std::shared_ptr<ast::Value> &sv_val) {
    Value val;
    if (auto int_lit = std::dynamic_pointer_cast<ast::IntLit>(sv_val)) {
        val.set_int(int_lit->val);
    } else if (auto float_lit = std::dynamic_pointer_cast<ast::FloatLit>(sv_val)) {
        val.set_float(float_lit->val);
    } else if (auto str_lit = std::dynamic_pointer_cast<ast::StringLit>(sv_val)) {
        val.set_str(str_lit->val);
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

// 在Analyze类中添加新函数
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