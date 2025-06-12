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

#include "execution/execution_defs.h"
#include "execution/execution_manager.h"
#include "record/rm.h"
#include "system/sm.h"
#include "common/context.h"
#include "plan.h"
#include "parser/parser.h"
#include "common/common.h"
#include "analyze/analyze.h"

class Planner {
   private:
    SmManager *sm_manager_;

    bool enable_nestedloop_join = true;
    bool enable_sortmerge_join = false;

   public:
    Planner(SmManager *sm_manager) : sm_manager_(sm_manager) {}


    std::shared_ptr<Plan> do_planner(std::shared_ptr<Query> query, Context *context);

    void set_enable_nestedloop_join(bool set_val) { enable_nestedloop_join = set_val; }
    
    void set_enable_sortmerge_join(bool set_val) { enable_sortmerge_join = set_val; }
    //void explain_plan(std::shared_ptr<Plan> plan, std::ostream& os, int indent = 0);
   private:
    std::shared_ptr<Query> logical_optimization(std::shared_ptr<Query> query, Context *context);
    std::shared_ptr<Plan> physical_optimization(std::shared_ptr<Query> query, Context *context);

    std::shared_ptr<Plan> make_one_rel(std::shared_ptr<Query> query);

    std::shared_ptr<Plan> generate_sort_plan(std::shared_ptr<Query> query, std::shared_ptr<Plan> plan);
    
    std::shared_ptr<Plan> generate_select_plan(std::shared_ptr<Query> query, Context *context);
    

    // int get_indexNo(std::string tab_name, std::vector<Condition> curr_conds);
    bool get_index_cols(std::string tab_name, std::vector<Condition>& curr_conds, std::vector<std::string>& index_col_names);

    ColType interp_sv_type(ast::SvType sv_type) {
        std::map<ast::SvType, ColType> m = {
            {ast::SV_TYPE_INT, TYPE_INT}, {ast::SV_TYPE_FLOAT, TYPE_FLOAT}, {ast::SV_TYPE_STRING, TYPE_STRING}};
        return m.at(sv_type);
    }
    void predicate_pushdown(std::shared_ptr<Query> query, Context *context);
    void projection_pushdown(std::shared_ptr<Query> query, Context *context);
    size_t get_table_cardinality(const std::string& tab_name);
    size_t estimate_join_size(const std::vector<size_t>& joined_indices,
                             size_t new_index,
                             const std::vector<std::string>& tables,
                             const std::vector<size_t>& cardinalities,
                             const std::vector<Condition>& conds);
    double calculate_join_selectivity(
    const std::string& lhs_tab, const std::string& lhs_col,
    const std::string& rhs_tab, const std::string& rhs_col,
    CompOp op);

    size_t estimate_ndv(const std::string& tab_name, const std::string& col_name);
    
};

// 索引选择结果结构
struct IndexSelectionResult {
    std::vector<std::string> index_col_names;
    bool found = false;
    size_t matched_length = 0;
    size_t equal_count = 0;
};

// 条件映射信息
struct ConditionMapping {
    std::unordered_set<std::string> available_columns;        // 可用的列名
    std::unordered_map<std::string, size_t> column_to_index;  // 列名到条件索引的映射
    std::unordered_map<std::string, size_t> duplicate_conditions; // 重复条件的映射
    
    ConditionMapping(const std::vector<Condition>& conditions) {
        available_columns.reserve(conditions.size());
        column_to_index.reserve(conditions.size());
        duplicate_conditions.reserve(2);
        
        for (size_t i = 0; i < conditions.size(); ++i) {
            const auto& col_name = conditions[i].lhs_col.col_name;
            if (available_columns.find(col_name) == available_columns.end()) {
                available_columns.insert(col_name);
                column_to_index[col_name] = i;
            } else {
                duplicate_conditions[col_name] = i;
            }
        }
    }
    
    bool hasColumn(const std::string& col_name) const {
        return available_columns.find(col_name) != available_columns.end();
    }
};