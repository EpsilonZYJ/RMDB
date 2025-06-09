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
    if (auto x = std::dynamic_pointer_cast<ast::SelectStmt>(parse))//select语句
    {
        /* 大致处理顺序： 表名-->聚合函数类型-->where-->group by-->sort/having */

        // 处理表名
        query->tables = std::move(x->tabs);
        /** TODO: 检查表是否存在 */
        for (auto &tab_name : query->tables) {
            if (!sm_manager_->db_.is_table(tab_name)) {
                throw TableNotFoundError(tab_name);
            }
        }
        // 处理target list，再target list中添加上表名，例如 a.id
        // // for (auto &sv_sel_col : x->cols) {
        // //     TabCol sel_col = {.tab_name = sv_sel_col->tab_name, .col_name = sv_sel_col->col_name};
        // //     query->cols.push_back(sel_col);
        // // }
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
            for (size_t i = 0; i < query->cols.size(); i++) {
                TabCol &sel_col = query->cols[i];
                if (query->agg_types[i] == AGG_COUNT && 
                    query->cols[i].tab_name.empty() && 
                    query->cols[i].col_name.empty()) 
                    continue; // 特判count(*)
                sel_col = check_column(all_cols, sel_col);  // 列元数据校验
            }
        }

        //处理where条件
        get_clause(x->conds, query->conds);

        // 处理group by条件
        for (auto &group_by: x->group_bys) 
            query->group_bys.emplace_back(TabCol{group_by->tab_name, group_by->col_name});
        for (auto &tab_col: query->group_bys) // 校验group by的列
            check_column(all_cols, tab_col);
        
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
            check_column(all_cols, tab_col);
            query->order_bys = std::move(tab_col);
        }

        get_having_clause(x->havings, query->havings);
        check_clause(query->tables, query->havings, true);
        check_clause(query->tables, query->conds);

        // 处理limit子句
        if(x->limit) {
            if (x->limit->limit_num < 0) {
                throw InternalError("LIMIT clause must be a non-negative integer.");
            }
            query->limit = x->limit->limit_num; // 处理limit
        } else {
            query->limit = -1; // 没有限制
        }
        
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
        
        check_clause({x->tab_name}, query->conds);
    } else if (auto x = std::dynamic_pointer_cast<ast::DeleteStmt>(parse)) {
        query->tables.push_back(x->tab_name);
        if (!sm_manager_->db_.is_table(x->tab_name)) {
            throw TableNotFoundError(x->tab_name);
        }
        get_clause(x->conds, query->conds);
        check_clause({x->tab_name}, query->conds);        
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

/* infer table name from column name */
TabCol Analyze::check_column(const std::vector<ColMeta> &all_cols, TabCol& target) {
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
        /* TODO: Make sure target column exists */
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
