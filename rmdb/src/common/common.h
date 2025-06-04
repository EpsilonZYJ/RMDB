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
#include "defs.h"
#include "record/rm_defs.h"


struct TabCol {
    std::string tab_name;
    std::string col_name;

    friend bool operator<(const TabCol &x, const TabCol &y) {
        return std::make_pair(x.tab_name, x.col_name) < std::make_pair(y.tab_name, y.col_name);
    }
};

struct Value {
    ColType type;  // type of value
    union {
        int int_val;      // int value
        float float_val;  // float value
    };
    std::string str_val;  // string value

    std::shared_ptr<RmRecord> raw;  // raw record buffer

    void set_int(int int_val_) {
        type = TYPE_INT;
        int_val = int_val_;
    }

    void set_float(float float_val_) {
        type = TYPE_FLOAT;
        float_val = float_val_;
    }

    void set_str(std::string str_val_) {
        type = TYPE_STRING;
        str_val = std::move(str_val_);
    }

    Value() = default; // TODO 默认构造函数不是很好的设计，可能会导致未定义行为

    // 拷贝构造
    Value(const Value &other) noexcept
        : type(other.type), raw(other.raw) {
        if (type == TYPE_INT) {
            int_val = other.int_val;
        } else if (type == TYPE_FLOAT) {
            float_val = other.float_val;
        } else if (type == TYPE_STRING) {
            str_val = other.str_val;
        }
    }

    // 移动构造
    Value(Value &&other) noexcept
        : type(other.type), raw(std::move(other.raw)) {
        if (type == TYPE_INT) {
            int_val = other.int_val;
            other.int_val = 0;
        } else if (type == TYPE_FLOAT) {
            float_val = other.float_val;
            other.float_val = 0.0f;
        } else if (type == TYPE_STRING) {
            str_val = std::move(other.str_val);
        }
    }

    // 拷贝赋值构造
    Value &operator=(const Value &other) noexcept {
        if (this != &other) {
            type = other.type;
            if (type == TYPE_INT) {
                int_val = other.int_val;
            } else if (type == TYPE_FLOAT) {
                float_val = other.float_val;
            } else if (type == TYPE_STRING) {
                str_val = other.str_val;
            }
            raw = other.raw;
        }
        return *this;
    }

    // 移动赋值构造
    Value &operator=(Value &&other) noexcept {
        if (this != &other) {
            type = other.type;
            if (type == TYPE_INT) {
                int_val = other.int_val;
                other.int_val = 0;
            } else if (type == TYPE_FLOAT) {
                float_val = other.float_val;
                other.float_val = 0.0f;
            } else if (type == TYPE_STRING) {
                str_val = std::move(other.str_val);
            }
            raw = std::move(other.raw);
        }
        return *this;
    }

    ~Value() = default;

    void init_raw(int len) {
        assert(raw == nullptr);
        raw = std::make_shared<RmRecord>(len); // 创建一个RmRecord对象
        if (type == TYPE_INT) {
            assert(len == sizeof(int));
            *(int *)(raw->data) = int_val;
        } else if (type == TYPE_FLOAT) {
            assert(len == sizeof(float));
            *(float *)(raw->data) = float_val;
        } else if (type == TYPE_STRING) {
            if (len < (int)str_val.size()) {
                throw StringOverflowError();
            }
            memset(raw->data, 0, len);
            memcpy(raw->data, str_val.c_str(), str_val.size());
        }
    }

    // TODO 这个函数会破坏原有的设计吗
    void set_raw(int len) {
        if (type == TYPE_INT) {
            assert(len == sizeof(int));
            *(int *)(raw->data) = int_val;
        } else if (type == TYPE_FLOAT) {
            assert(len == sizeof(float));
            *(float *)(raw->data) = float_val;
        } else if (type == TYPE_STRING) {
            if (len < (int)str_val.size()) {
                throw StringOverflowError();
            }
            memset(raw->data, 0, len);
            memcpy(raw->data, str_val.c_str(), str_val.size());
        }
    }
    
    bool operator==(const struct Value& rhs) const {
        //! 以下代码有点问题，因为如果类型不一样，直接比较会出错
        // if (type != rhs.type) return false;

        // switch(type) {
        //     case TYPE_INT:    return int_val == rhs.int_val;
        //     case TYPE_FLOAT:  return float_val == rhs.float_val;
        //     case TYPE_STRING: return str_val == rhs.str_val;
        //     default:          return false;
        // }
        //TODO 也许用强制类型转换的方法会好一些，符合编译器的做法
        if((type==TYPE_INT)&&(rhs.type==TYPE_FLOAT)){
            return int_val == rhs.float_val;
        }else if((type==TYPE_FLOAT)&&(rhs.type==TYPE_INT)){
            return float_val == rhs.int_val;
        }else if((type==TYPE_INT)&&(rhs.type==TYPE_INT)){
            return int_val == rhs.int_val;
        }else if((type==TYPE_FLOAT)&&(rhs.type==TYPE_FLOAT)){
            return float_val == rhs.float_val;
        }else if((type==TYPE_STRING)&&(rhs.type==TYPE_STRING)){
            return str_val == rhs.str_val;
        }else{
            throw InternalError("Invalid value type");
        }
    }

    bool operator!=(const struct Value& rhs) const {
        return !(*this == rhs);
    }

    bool operator<(const Value &rhs) const {
        // if (type != rhs.type) 
        //     throw IncompatibleTypeError(coltype2str(type), coltype2str(rhs.type));
        // switch (type) {
        //     case TYPE_INT:    return int_val < rhs.int_val;
        //     case TYPE_FLOAT:  return float_val < rhs.float_val;
        //     case TYPE_STRING: return str_val.compare(rhs.str_val) < 0;
        //     default: throw InternalError("Unexpected value type");
        // }
        //TODO 也许用强制类型转换的方法会好一些，符合编译器的做法
        if((type==TYPE_INT)&&(rhs.type==TYPE_FLOAT)){
            return int_val < rhs.float_val;
        }else if((type==TYPE_FLOAT)&&(rhs.type==TYPE_INT)){
            return float_val < rhs.int_val;
        }else if((type==TYPE_INT)&&(rhs.type==TYPE_INT)){
            return int_val < rhs.int_val;
        }else if((type==TYPE_FLOAT)&&(rhs.type==TYPE_FLOAT)){
            return float_val < rhs.float_val;
        }else if((type==TYPE_STRING)&&(rhs.type==TYPE_STRING)){
            return str_val < rhs.str_val;
        }else{
            throw InternalError("Invalid value type");
        }
    }

    bool operator>(const Value &rhs) const { return rhs < *this; }

    bool operator<=(const Value &rhs) const { return !(rhs < *this); }

    bool operator>=(const Value &rhs) const { return !(*this < rhs); }

    void value_cast(ColType new_type) {
        if (type == new_type) return;
        if (type == TYPE_INT && new_type == TYPE_FLOAT) float_val = int_val;
        else if (type == TYPE_FLOAT && new_type == TYPE_INT) int_val = float_val;
        else if(type == TYPE_STRING && new_type == TYPE_INT) 
            sscanf(str_val.c_str(), "%d", &int_val);
        else if(type == TYPE_STRING && new_type == TYPE_FLOAT)
            sscanf(str_val.c_str(), "%f", &float_val);
        else throw IncompatibleTypeError(coltype2str(type), coltype2str(new_type));
        type = new_type;

        if (!raw) return;

        int len;
        switch (type){
            case TYPE_INT:    len = sizeof(int);        break;
            case TYPE_FLOAT:  len = sizeof(float);      break;
            case TYPE_STRING: len = str_val.size() + 1; break;
        }
        set_raw(len);
    }
};

enum CompOp { OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE };

struct Condition {
    TabCol lhs_col;   // left-hand side column
    CompOp op;        // comparison operator
    bool is_rhs_val;  // true if right-hand side is a value (not a column)
    TabCol rhs_col;   // right-hand side column
    Value rhs_val;    // right-hand side value
    bool is_join_cond = false; // 是否为JOIN条件

    bool check(const Value& lhs, const Value& rhs) const {
        switch (op) {
            case OP_EQ: return lhs == rhs;
            case OP_NE: return lhs != rhs;
            case OP_LT: return lhs < rhs;
            case OP_GT: return lhs > rhs;
            case OP_LE: return lhs <= rhs;
            case OP_GE: return lhs >= rhs;
            default: throw InternalError("Unexpected operator");
        }
    }

    
};

struct SetClause {
    TabCol lhs;
    Value rhs;
};