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
    std::string year;  // YYYY
    std::string month; // MM
    std::string day;   // DD

    Date(std::string date) {
        if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
            throw InvalidArgumentError(date, "Invalid date format, expected YYYY-MM-DD");
        }
        year = date.substr(0, 4);
        month = date.substr(5, 2);
        day = date.substr(8, 2);
    }

    Date(int y, int m, int d) {
        if (y < 0 || m < 1 || m > 12 || d < 1 || d > 31) {
            throw InvalidArgumentError("Date", "Invalid date values");
        }
        year = std::to_string(y);
        month = (m < 10 ? "0" : "") + std::to_string(m);
        day = (d < 10 ? "0" : "") + std::to_string(d);
    }

    std::string toString() const {
        return year + "-" + month + "-" + day;
    }

    friend std::ostream &operator<<(std::ostream &os, const Date &date) {
        os << date.year << "-" << date.month << "-" << date.day;
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
