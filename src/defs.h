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

#include <iostream>
#include <map>
#include <errors.h>
//#include <regex>

// 此处重载了<<操作符，在ColMeta中进行了调用
template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
std::ostream &operator<<(std::ostream &os, const T &enum_val) {
    os << static_cast<int>(enum_val);
    return os;
}

template<typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type>
std::istream &operator>>(std::istream &is, T &enum_val) {
    int int_val;
    is >> int_val;
    enum_val = static_cast<T>(int_val);
    return is;
}

/* 对每个记录的唯一标识 */
struct Rid {
    int page_no;
    int slot_no;

    friend bool operator==(const Rid &x, const Rid &y) {
        return x.page_no == y.page_no && x.slot_no == y.slot_no;
    }

    friend bool operator!=(const Rid &x, const Rid &y) { return !(x == y); }
};

struct Date {
    std::string date; 

    Date(std::string date_) : date(std::move(date_)) {
        if (!check_valid()) {
            throw InvalidArgumentError("date", "Invalid date");
        }
    }

    std::string toString() const {
        return date;
    }

    bool check_valid() const {
        return check_valid(date);
    }

 
    static bool check_valid(const std::string &date) {
        // 首先检查长度是否正确（格式为 "YYYY-MM-DD HH:MM:SS" 应为19个字符）
        if (date.length() != 19) return false;
        
        // 检查分隔符位置
        if (date[4] != '-' || date[7] != '-' || date[10] != ' ' || 
            date[13] != ':' || date[16] != ':') return false;
        
        // 检查所有应该是数字的位置
        for (int i = 0; i < 19; i++) {
            if (i == 4 || i == 7 || i == 10 || i == 13 || i == 16) continue; // 跳过分隔符
            if (!isdigit(date[i])) return false;
        }
        
        try {
            // 解析年月日时分秒
            int year = std::stoi(date.substr(0, 4));
            int month = std::stoi(date.substr(5, 2));
            int day = std::stoi(date.substr(8, 2));
            int hour = std::stoi(date.substr(11, 2));
            int minute = std::stoi(date.substr(14, 2));
            int second = std::stoi(date.substr(17, 2));
            
            // 检查范围
            if (year < 0 || month < 1 || month > 12 || day < 1) return false;
            
            // 计算每月天数
            int max_days = 31;
            if (month == 4 || month == 6 || month == 9 || month == 11) {
                max_days = 30;
            } else if (month == 2) {
                bool is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
                max_days = is_leap ? 29 : 28;
            }
            
            if (day > max_days) return false;
            if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) return false;
            
            return true;
        } catch (const std::exception&) {
            return false; // 转换失败
        }
    }

    friend bool operator==(const Date &x, const Date &y) {
        return x.date == y.date;
    }

    friend bool operator!=(const Date &x, const Date &y) {
        return !(x == y);
    }

    friend bool operator<(const Date &x, const Date &y) {
        return x.date < y.date;
    }

    friend bool operator>(const Date &x, const Date &y) {
        return x.date > y.date;
    }

    friend bool operator<=(const Date &x, const Date &y) {
        return x.date <= y.date;
    }

    friend bool operator>=(const Date &x, const Date &y) {
        return x.date >= y.date;
    }

    friend std::ostream &operator<<(std::ostream &os, const Date &date) {
        os << date.date;
        return os;
    }
};

enum ColType {
    TYPE_INT, 
    TYPE_FLOAT, 
    TYPE_STRING,
    TYPE_DATE
};

inline bool value_type_match(ColType type1, ColType type2) {
    if (type1 == type2) return true;
    if (type1 == ColType::TYPE_INT && type2 == ColType::TYPE_FLOAT) return true;
    if (type1 == ColType::TYPE_FLOAT && type2 == ColType::TYPE_INT) return true;
    return false;
}

inline std::string coltype2str(ColType type) {
    std::map<ColType, std::string> m = {
            {TYPE_INT,    "INT"},
            {TYPE_FLOAT,  "FLOAT"},
            {TYPE_STRING, "STRING"},
            {TYPE_DATE,   "DATE"}
    };
    return m.at(type);
}

class RecScan {
public:
    virtual ~RecScan() = default;

    virtual void next() = 0;

    virtual bool is_end() const = 0;

    virtual Rid rid() const = 0;
};
