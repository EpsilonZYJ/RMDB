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
//#include <regex>
#include "defs.h"
#include "record/rm_defs.h"
#include "parser/parser.h"


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

    // Date类型与字符串类型公用str_val（因为底层都是字符串存储）
    void set_date(std::string date) {
        type = TYPE_DATE;
        str_val = std::move(date);
    }
    void set_date(Date date) {
        type = TYPE_DATE;
        str_val = date.toString();  // YYYY-MM-DD
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
        } else if (type == TYPE_DATE) { // 为了易读，将Date与string的比较逻辑拆分开
            str_val = other.str_val;  // Date类型也存储在str_val中
        } else {
            throw InternalError("Invalid value type");
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
        } else if (type == TYPE_DATE) {
            str_val = std::move(other.str_val);  // Date类型也存储在str_val中
        } else {
            throw InternalError("Invalid value type");
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
            } else if (type == TYPE_DATE) {
                str_val = other.str_val;  // Date类型也存储在str_val中
            } else {
                throw InternalError("Invalid value type");
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
            } else if (type == TYPE_DATE) {
                str_val = std::move(other.str_val);  // Date类型也存储在str_val中
            } else {
                throw InternalError("Invalid value type");
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
                std::cout << "字段内容: [" << str_val << "], 长度: " << str_val.size() << std::endl;

                // 十六进制打印
                std::cout << "十六进制: ";
                for (size_t i = 0; i < str_val.size(); ++i) {
                    printf("%02X ", (unsigned char)str_val[i]);
                }
                throw StringOverflowError();
            }
            memset(raw->data, 0, len);
            memcpy(raw->data, str_val.c_str(), str_val.size());
        } else if (type == TYPE_DATE) {
            if (len < (int)str_val.size()) {
                throw StringOverflowError();
            }
            memset(raw->data, 0, len);
            memcpy(raw->data, str_val.c_str(), str_val.size());
        } else {
            throw InternalError("Invalid value type");
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
                std::cerr << "右侧： " << str_val<< std::endl;
                std::cerr << "右侧长度： " << str_val.size() << " > " << len << std::endl;
                throw StringOverflowError();
            }
            memset(raw->data, 0, len);
            memcpy(raw->data, str_val.c_str(), str_val.size());
        } else if (type == TYPE_DATE) {
            if (len < (int)str_val.size()) {
                throw StringOverflowError();
            }
            memset(raw->data, 0, len);
            memcpy(raw->data, str_val.c_str(), str_val.size());
        } else {
            throw InternalError("Invalid value type");
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
        }else if((type==TYPE_DATE)&&(rhs.type==TYPE_DATE)){
            return str_val == rhs.str_val; // Date类型也存储在str_val中
        }else if((type==TYPE_STRING)&&(rhs.type==TYPE_DATE) ||
                (type==TYPE_DATE)&&(rhs.type==TYPE_STRING)){
            // 如果左侧是字符串，右侧是日期，则比较字符串
            return str_val == rhs.str_val; // Date类型也存储在str_val中
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
        }else if((type==TYPE_DATE)&&(rhs.type==TYPE_DATE)){
            return str_val < rhs.str_val; // Date类型也存储在str_val中
        }else if((type==TYPE_STRING)&&(rhs.type==TYPE_DATE) ||
                (type==TYPE_DATE)&&(rhs.type==TYPE_STRING)){
            // 如果左侧是字符串，右侧是日期，则比较字符串
            return str_val < rhs.str_val; // Date类型也存储在str_val中
        }else{
            throw InternalError("Invalid value type");
        }
    }

    bool operator>(const Value &rhs) const { return rhs < *this; }

    bool operator<=(const Value &rhs) const { return !(rhs < *this); }

    bool operator>=(const Value &rhs) const { return !(*this < rhs); }

    Value operator+(const Value &rhs) {
        if (type != rhs.type) 
            throw IncompatibleTypeError(coltype2str(type), coltype2str(rhs.type));

        if (type == TYPE_INT) {
            int_val = int_val + rhs.int_val;
        } else if (type == TYPE_FLOAT) {
            float_val = float_val + rhs.float_val;
        } else {
            throw InternalError("Unexpected value type");
        }

        if (!raw) return *this;

        int len;
        switch (type) {
            case TYPE_INT:    len = sizeof(int);        break;
            case TYPE_FLOAT:  len = sizeof(float);      break;
            case TYPE_STRING: len = str_val.size() + 1; break;
        }
        set_raw(len);
        
        return *this;
    }

    Value operator/(int devisor) {
        assert(devisor != 0 && "Division by zero is not allowed");

        if (type == TYPE_INT) int_val /= devisor;
        else if (type == TYPE_FLOAT) float_val /= devisor;
        else throw InternalError("Unexpected value type");

        if (!raw) return *this;

        int len;
        switch (type) {
            case TYPE_INT:    len = sizeof(int);        break;
            case TYPE_FLOAT:  len = sizeof(float);      break;
            case TYPE_STRING: len = str_val.size() + 1; break;
        }
        set_raw(len);
        
        return *this;
    }

    void value_cast(ColType new_type) {
        if (type == new_type) return;
        if (type == TYPE_INT && new_type == TYPE_FLOAT) float_val = int_val;
        else if (type == TYPE_FLOAT && new_type == TYPE_INT) int_val = float_val;
        else if(type == TYPE_STRING && new_type == TYPE_INT) 
            sscanf(str_val.c_str(), "%d", &int_val);
        else if(type == TYPE_STRING && new_type == TYPE_FLOAT)
            sscanf(str_val.c_str(), "%f", &float_val);
        else if(type == TYPE_STRING && new_type == TYPE_DATE) {
            if (!Date::check_valid(str_val)) {
                throw InvalidArgumentError(str_val, "Invalid date format, expected YYYY-MM-DD");
            }
        }
        else throw IncompatibleTypeError(coltype2str(type), coltype2str(new_type));
        type = new_type;

        if (!raw) return;

        int len;
        switch (type){
            case TYPE_INT:    len = sizeof(int);        break;
            case TYPE_FLOAT:  len = sizeof(float);      break;
            case TYPE_STRING: len = str_val.size() + 1; break;
            case TYPE_DATE:   len = str_val.size() + 1; break;
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
    bool is_semi_join = false;
    bool is_anti_join = false;

    /* 新增 */
    AggType agg_type = NO_AGG; // aggregation type, default is NO_AGG

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
    bool is_expr = false;   // 是否为表达式更新
    TabCol ref_col;         // 引用的列
    char op_type = '\0';    // 运算符类型(+,-,*,/)
};