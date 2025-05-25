#ifndef CHECK_CONDITION_H
#define CHECK_CONDITION_H

#include "check_condition.h"

bool check_condition(const RmRecord& record, TabMeta& tab_, const Condition& cond) {
    // 获取列信息

    // 获取左操作数的值
    Value lhs_value;
    int lhs_offset = tab_.get_col(cond.lhs_col.col_name)->offset;
    lhs_value.type = tab_.get_col(cond.lhs_col.col_name)->type;
    int len = tab_.get_col(cond.lhs_col.col_name)->len;
    lhs_value.init_raw(len);
    memcpy(lhs_value.raw->data, record.data + lhs_offset, len);

    // 获取右操作数的值
    Value rhs_value;
    if (cond.is_rhs_val) {
        rhs_value = cond.rhs_val;
    } else {
        int rhs_offset = tab_.get_col(cond.rhs_col.col_name)->offset;
        rhs_value.type = tab_.get_col(cond.rhs_col.col_name)->type;
        int len = tab_.get_col(cond.rhs_col.col_name)->len;
        rhs_value.init_raw(len);
        memcpy(rhs_value.raw->data, record.data + rhs_offset, len);
    }

    return cond.check(lhs_value, rhs_value);
}

bool check_condition(const RmRecord& record, TabMeta& tab_, const std::vector<Condition>& conds) {
    for (const auto& cond : conds) 
        if (!check_condition(record, tab_, cond))
            return false;

    return true;
}
#endif