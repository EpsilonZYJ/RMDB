/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "parser/parser.h"
#include "system/sm.h"
#include "common/common.h"
#include <map>
#include <set>
class Query{
    public:
    std::shared_ptr<ast::TreeNode> parse;//抽象语法树
    // TODO jointree
    // where条件
    std::vector<Condition> conds;
    // 投影列
    std::vector<TabCol> cols;
    // 表名
    std::vector<std::string> tables;
    // update 的set 值
    std::vector<SetClause> set_clauses;
    //insert 的values值
    std::vector<Value> values;
    // 聚合类型
    std::vector<AggType> agg_types;
    // 别名
    std::vector<std::string> alias;
    // group by
    std::vector<TabCol> group_bys;
    // having 条件
    std::vector<Condition> havings;
    // order by
    TabCol order_bys;
    // limit n
    int limit = -1; // -1表示没有限制
    
    //join的连接条件
    std::vector<std::vector<Condition>> join_conds;
    Query(){}
    bool is_explain = false; // 是否为explain语句
    std::map<std::string, std::set<std::string> > table_required_cols;//储存每个表需要的列
    std::map<std::string, std::string> tab_alias_map; // 表名到别名的映射
};

class Analyze
{
private:
    SmManager *sm_manager_;
    bool current_is_semi_join_ = false;
    std::set<std::string> current_semi_join_left_tables_;
public:
    Analyze(SmManager *sm_manager) : sm_manager_(sm_manager){}
    ~Analyze(){}

    std::shared_ptr<Query> do_analyze(std::shared_ptr<ast::TreeNode> root);
    void set_semi_join_info(bool is_semi_join, const std::set<std::string>& left_tables) {
        current_is_semi_join_ = is_semi_join;
        current_semi_join_left_tables_ = left_tables;
    }

private:
    TabCol check_column(const std::vector<ColMeta> &all_cols, TabCol target, 
        const std::map<std::string, std::string> &tab_alias_map = {},
    bool is_semi_join = false, const std::set<std::string> &left_tables = {});
    void get_all_cols(const std::vector<std::string> &tab_names, std::vector<ColMeta> &all_cols);
    void get_clause(const std::vector<std::shared_ptr<ast::BinaryExpr>> &sv_conds, std::vector<Condition> &conds);
    void check_clause(const std::vector<std::string> &tab_names, std::vector<Condition> &conds, bool check_having=false);
    void check_clause(const std::vector<std::string> &tab_names, std::vector<Condition> &conds,const std::map<std::string, std::string> &tab_alias_map);
    void get_having_clause(const std::vector<std::shared_ptr<ast::HavingExpr> > &having_conds, std::vector<Condition> &conds);
    Value convert_sv_value(const std::shared_ptr<ast::Value> &sv_val);
    CompOp convert_sv_comp_op(ast::SvCompOp op);
    bool value_type_match(ColType type1, ColType type2);
    void analyze_table_refs(const std::vector<std::string> &tab_refs, std::shared_ptr<Query> query);
};

