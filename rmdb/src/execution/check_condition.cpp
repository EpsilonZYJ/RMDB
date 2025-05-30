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
    
    if (lhs_value.type == TYPE_INT) {
        lhs_value.int_val = *(int*)lhs_value.raw->data;
    } else if (lhs_value.type == TYPE_FLOAT) {
        lhs_value.float_val = *(float*)lhs_value.raw->data;
    } else if (lhs_value.type == TYPE_STRING) {
        lhs_value.str_val = std::string(lhs_value.raw->data);
    }
    
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
// TODO 这对吗？
        if (lhs_value.type == TYPE_INT) {
            lhs_value.int_val = *(int*)lhs_value.raw->data;
        } else if (lhs_value.type == TYPE_FLOAT) {
            lhs_value.float_val = *(float*)lhs_value.raw->data;
        } else if (lhs_value.type == TYPE_STRING) {
            lhs_value.str_val = std::string(lhs_value.raw->data);
        }
    }

    return cond.check(lhs_value, rhs_value);
}

/* 
    @param lhs_col: cond的左操作数的列元数据
    @param rhs_col: cond的右操作数的列元数据
*/
bool check_condition(const RmRecord& record, 
                    const ColMeta& lhs_col,
                    const ColMeta& rhs_col,
                    const Condition& cond) {

    // 获取左操作数的值
    Value lhs_value;
    int lhs_offset = lhs_col.offset;
    lhs_value.type = lhs_col.type;
    int len = lhs_col.len;
    lhs_value.init_raw(len);
    memcpy(lhs_value.raw->data, record.data + lhs_offset, len);
    
    if (lhs_value.type == TYPE_INT) {
        lhs_value.int_val = *(int*)lhs_value.raw->data;
    } else if (lhs_value.type == TYPE_FLOAT) {
        lhs_value.float_val = *(float*)lhs_value.raw->data;
    } else if (lhs_value.type == TYPE_STRING) {
        lhs_value.str_val = std::string(lhs_value.raw->data);
    }
    
    // 获取右操作数的值
    Value rhs_value;
    if (cond.is_rhs_val) {
        rhs_value = cond.rhs_val;
    } else {
        int rhs_offset = rhs_col.offset;
        rhs_value.type = lhs_col.type;
        int len = rhs_col.len;
        rhs_value.init_raw(len);
        memcpy(rhs_value.raw->data, record.data + rhs_offset, len);
// TODO 这对吗？
        if (rhs_value.type == TYPE_INT) {
            rhs_value.int_val = *(int*)rhs_value.raw->data;
        } else if (rhs_value.type == TYPE_FLOAT) {
            rhs_value.float_val = *(float*)rhs_value.raw->data;
        } else if (rhs_value.type == TYPE_STRING) {
            rhs_value.str_val = std::string(rhs_value.raw->data);
        }
    }

    return cond.check(lhs_value, rhs_value);
}


bool check_condition(const RmRecord& record, TabMeta& tab_, const std::vector<Condition>& conds) {
    for (const auto& cond : conds) 
        if (!check_condition(record, tab_, cond))
            return false;

    return true;
}

bool check_condition(const RmRecord& record, 
                    const std::vector<ColMeta>& left_cols,
                    const std::vector<ColMeta>& right_cols,
                    const std::vector<Condition>& conds) {
    for (int i = 0; i < conds.size(); ++i) 
        if (!check_condition(record, left_cols[i], right_cols[i], conds[i]))
            return false;

    return true;
}
#endif