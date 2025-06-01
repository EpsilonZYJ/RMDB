#pragma once

#include "common/common.h"
#include <system/sm.h>
bool check_condition(const RmRecord& record, TabMeta& tab_, const Condition& cond);

bool check_condition(const RmRecord& record, TabMeta& tab_, const std::vector<Condition>& conds);

bool check_condition(const RmRecord& record, 
                    const ColMeta& lhs_col,
                    const ColMeta& rhs_col,
                    const Condition& cond);

bool check_condition(const RmRecord& record, 
                     const std::vector<ColMeta>& left_cols,
                     const std::vector<ColMeta>& right_cols,
                     const std::vector<Condition>& conds);
                     #pragma once

bool check_condition(const RmRecord* record, const Condition* cond, 
                     const std::vector<ColMeta>& cols, 
                     void* left_tuple, void* right_tuple);
