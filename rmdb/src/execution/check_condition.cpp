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

bool check_condition(const RmRecord* record, const Condition* cond,
        const std::vector<ColMeta>& cols,
        void* left_tuple, void* right_tuple) {
    if (!record || !cond) return false;

    // 在cols中找到左操作数对应的列
    const ColMeta* lhs_col = nullptr;
    for (const auto& col : cols) {
        if (col.tab_name == cond->lhs_col.tab_name && 
        col.name == cond->lhs_col.col_name) {
        lhs_col = &col;
        break;
    }
    }

     // 如果找不到左侧列，条件不成立
     if (!lhs_col) {
        std::cerr << "找不到列: " << cond->lhs_col.tab_name << "." << cond->lhs_col.col_name << std::endl;
        return false;
    }

    // 获取左操作数的值
    Value lhs_value;
    lhs_value.type = lhs_col->type;
    int len = lhs_col->len;
    // 防御性检查：确保偏移量有效
    if (lhs_col->offset + len > record->size) {
        std::cerr << "列偏移无效: " << cond->lhs_col.tab_name << "." << cond->lhs_col.col_name 
                  << " offset=" << lhs_col->offset << " len=" << len 
                  << " record_size=" << record->size << std::endl;
        return false;
    }
    lhs_value.init_raw(len);
    memcpy(lhs_value.raw->data, record->data + lhs_col->offset, len);

    // 设置实际值
    if (lhs_value.type == TYPE_INT) {
    lhs_value.int_val = *(int*)lhs_value.raw->data;
    } else if (lhs_value.type == TYPE_FLOAT) {
    lhs_value.float_val = *(float*)lhs_value.raw->data;
    } else if (lhs_value.type == TYPE_STRING) {
    lhs_value.str_val = std::string(lhs_value.raw->data);
    }

    // 获取右操作数的值
    Value rhs_value;
    if (cond->is_rhs_val) {
    // 右操作数是常量
    rhs_value = cond->rhs_val;
    } else {
    // 右操作数是列
    const ColMeta* rhs_col = nullptr;
    for (const auto& col : cols) {
    if (col.tab_name == cond->rhs_col.tab_name && 
    col.name == cond->rhs_col.col_name) {
    rhs_col = &col;
    break;
    }
    }

    if (!rhs_col) return false;  // 找不到对应的列

    rhs_value.type = rhs_col->type;
    int rhs_len = rhs_col->len;
    rhs_value.init_raw(rhs_len);
    memcpy(rhs_value.raw->data, record->data + rhs_col->offset, rhs_len);

    // 设置实际值
    if (rhs_value.type == TYPE_INT) {
    rhs_value.int_val = *(int*)rhs_value.raw->data;
    } else if (rhs_value.type == TYPE_FLOAT) {
    rhs_value.float_val = *(float*)rhs_value.raw->data;
    } else if (rhs_value.type == TYPE_STRING) {
    rhs_value.str_val = std::string(rhs_value.raw->data);
    }
    }

    // 检查条件是否满足
    return cond->check(lhs_value, rhs_value);
    }
