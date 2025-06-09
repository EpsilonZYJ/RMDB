#pragma once

#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"
#include "check_condition.h"

struct AggregateKey {
    std::vector<Value> grouping_key; // Group by values
};

struct AggregateVal {
    std::vector<Value> values;
    std::vector<Value> having_values; // Values for HAVING clause
    int count{1}; // 计数器，用于AVG  1表示当前算作一个记录
};

struct AggregateKeyEqual {
    bool operator()(const AggregateKey &lhs, const AggregateKey &rhs) const {
        return lhs.grouping_key == rhs.grouping_key;    
    }
};

struct AggregateKeyHash {
    inline void hash_combine(std::size_t &seed, std::size_t value) const {
        // Magic constant from Boost
        seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    std::size_t operator()(const AggregateKey &key) const {
        std::size_t hash = 0;
        for (const auto &value: key.grouping_key) {
            switch (value.type) {
                case TYPE_INT:
                    hash_combine(hash, std::hash<int>()(value.int_val));
                    break;
                case TYPE_FLOAT:
                    hash_combine(hash, std::hash<float>()(value.float_val));
                    break;
                case TYPE_STRING:
                    hash_combine(hash, std::hash<std::string>()(value.str_val));
                    break;
            }
        }
        return hash;
    }
};

class AggregateHashTable {
private:
    // 使用函数表封装不同聚合类型的处理逻辑
    using AggregateFunction = std::function<void(Value&, const Value&)>;
    std::unordered_map<AggType, AggregateFunction> agg_functions_;
    void initAggregateFunctions() {
        agg_functions_[AGG_COUNT] = [](Value &result, const Value &) {
            result.int_val += 1;
        };
        agg_functions_[AGG_MAX] = [](Value &lhs, const Value &rhs) {
            if (rhs > lhs) lhs = rhs;
        };
        agg_functions_[AGG_MIN] = [](Value &lhs, const Value &rhs) {
            if (rhs < lhs) lhs = rhs;
        };
        agg_functions_[AGG_SUM] = [](Value &lhs, const Value &rhs) {
            lhs = lhs + rhs; 
        };
        agg_functions_[AGG_AVG] = [](Value &lhs, const Value &rhs) {
            lhs = lhs + rhs; // 先累加，随后计算平均
        };
    }

    AggregateVal initAggregateVal(const AggregateVal &input) {
        std::vector<Value> values;
        std::vector<Value> having_values;
        for (std::size_t i = 0; i < agg_types_.size(); ++i) {
            switch (agg_types_[i]) {
                case AGG_COUNT: {
                    Value v;
                    v.set_int(1);
                    v.init_raw(sizeof(int));
                    values.emplace_back(v);
                    break;
                }
                case AGG_MAX: case AGG_MIN: case AGG_SUM: case AGG_AVG:
                    values.emplace_back(input.values[i]);
                    break;
                case NO_AGG:
                    values.emplace_back();
                    break;
                default:
                    throw InternalError("Unexpected aggregate type!");
            }
        }

        for (std::size_t i = 0; i < having_conds_.size(); ++i) {
            switch (having_conds_[i].agg_type) {
                case AGG_COUNT: {
                    Value v;
                    v.set_int(1);
                    v.init_raw(sizeof(int));
                    having_values.emplace_back(v);
                    break;
                }
                case AGG_MAX: case AGG_MIN: case AGG_SUM: case AGG_AVG:
                    having_values.emplace_back(input.having_values[i]);
                    break;
                default:
                    throw InternalError("Unexpected aggregate type!");
            }
        }

        return {values, having_values};
    }

    std::unordered_map<AggregateKey, AggregateVal, AggregateKeyHash, AggregateKeyEqual> hash_table_;


public:
    const std::vector<AggType> &agg_types_;
    const std::vector<Condition> &having_conds_;
    AggregateHashTable(
            const std::vector<AggType> &agg_types,
            const std::vector<Condition> &having_conds
        ) : agg_types_(agg_types), having_conds_(having_conds) {
        initAggregateFunctions();
    }

    // 提供迭代器接口
    auto begin() const { return hash_table_.begin(); }
    auto begin() { return hash_table_.begin(); }
    auto end() const { return hash_table_.end(); }
    auto end() { return hash_table_.end(); }
    auto find(const AggregateKey &key) const { return hash_table_.find(key); }
    auto size() const { return hash_table_.size(); }
    void clear() { hash_table_.clear(); }

    // 以下提供对哈希表的基本操作 

    // 检查键是否存在
    bool contains(const AggregateKey &key) const {
        return hash_table_.find(key) != hash_table_.end();
    }

    // 安全地获取值
    const AggregateVal* get(const AggregateKey &key) const {
        auto it = hash_table_.find(key);
        return it != hash_table_.end() ? &it->second : nullptr;
    }

    // 合并聚合值
    void combineAggregateVal(AggregateVal *l, const AggregateVal &r) {
        // 处理SELECT列表中的聚合
        for (std::size_t i = 0; i < agg_types_.size(); i++) {
            if (agg_types_[i] != NO_AGG && agg_functions_.count(agg_types_[i]) > 0) {
                agg_functions_[agg_types_[i]](l->values[i], r.values[i]);
            }
        }
        
        // 处理HAVING子句中的聚合
        for (std::size_t i = 0; i < having_conds_.size(); i++) {
            if (having_conds_[i].agg_type != NO_AGG && 
                agg_functions_.count(having_conds_[i].agg_type) > 0) {
                    agg_functions_[having_conds_[i].agg_type](
                        l->having_values[i], 
                        r.having_values[i]
                    );
            }
        }

        // 更新计数器
        l->count += r.count;
    }

    void insert(const AggregateKey &agg_key, const AggregateVal &agg_val) {
        if (hash_table_.count(agg_key) == 0) 
            hash_table_.emplace(agg_key, initAggregateVal(agg_val));
        else 
            combineAggregateVal(&hash_table_[agg_key], agg_val);
    }
};

/**
# AggregateExecutor成员变量详解 - 以实例说明

假设我们执行以下SQL查询：
```sql
SELECT dept_id, COUNT(salary) as emp_count
FROM employees 
GROUP BY dept_id 
HAVING COUNT(*) > 5;
```

## 基础表和输入数据
假设`employees`表有以下列：
- id (int)
- name (string)
- dept_id (int)
- salary (float)
## 主要成员变量含义
### 输入与上游相关
- **`prev_`**: 上游执行器的指针，提供employees表的扫描结果
- **`cols_`**: 从上游执行器接收的列元数据，包含[id, name, dept_id, salary]的定义
- **`rm_record_`**: 当前正在处理的记录，例如某位员工的完整记录
### 聚合配置
- **`sel_cols_`**: 输出列的元数据，包含[dept_id, COUNT(salary)]
- **`agg_types_`**: 每个输出列的聚合类型，这里是[AGG_COL, AGG_COUNT]
- **`group_bys_`**: GROUP BY列元数据，这里是[dept_id]
- **`having_cols_`**: HAVING子句中的列元数据，这里与COUNT(*)对应
- **`having_conds_`**: HAVING条件，描述"COUNT(*) > 5"
### 聚合执行
- **`ht_`**: 哈希表，用于分组计算。键为不同的dept_id值，值为聚合结果
- **`it_`**: 当前正在处理的哈希表迭代器位置
- **`has_group_col_`**: true，因为dept_id是非聚合列，出现在SELECT中
- **`is_empty_table_`**: 标记是否在处理空表的COUNT特殊情况
### 输出相关
- **`len_`**: 输出记录的总长度，这里是dept_id长度(4字节) + COUNT结果长度(4字节) = 8字节

## 执行流程示例
1. `beginTuple`执行时:
   - 遍历所有员工记录
   - 为每条记录提取dept_id作为键
   - 根据dept_id将记录分组，累计COUNT值
   - 最终`ht_.hash_table_`内容可能是：
     ```
     {dept_id:101} -> {count:8}
     {dept_id:102} -> {count:3}
     {dept_id:103} -> {count:12}
     ```
   - `it_`指向第一个满足HAVING条件的组(dept_id:101)
2. `Next`返回时:
   - 组装输出记录，包含dept_id和count值
   - 例如：{dept_id:101, count:8}
3. `nextTuple`执行时:
   - `it_`移动到下一个满足条件的组(dept_id:103)
   - 跳过不满足HAVING条件的组(dept_id:102)
 */
class AggregateExecutor : public AbstractExecutor {
private:
    std::unique_ptr<AbstractExecutor> prev_;  // 前置执行器
    
    // 元数据
    std::vector<ColMeta> cols_;       // 输入列元数据
    std::vector<ColMeta> sel_cols_;   // 选择列元数据
    std::vector<ColMeta> having_cols_;  // HAVING列元数据
    std::vector<ColMeta> group_bys_;  // GROUP BY列元数据
    
    // 聚合类型和条件
    std::vector<AggType> agg_types_; // 长度与sel_cols_相同，表示每列的聚合类型
    std::vector<Condition> having_conds_;
    
    // 哈希聚合表
    AggregateHashTable ht_;
    std::unordered_map<AggregateKey, AggregateVal, AggregateKeyHash, AggregateKeyEqual>::iterator ht_iter_;
    
    // 状态标志
    bool has_group_col_{false};
    bool is_empty_table_{false};
    
    // 其他字段
    size_t len_{0};  // 结果记录长度
    std::unique_ptr<RmRecord> rm_record_;  // 当前记录
    
    // 辅助方法
    
    // 从给定记录中提取给定字段的值
    Value extractValue(const RmRecord* record, const ColMeta& col) {
        Value v;
        switch (col.type) {
            case TYPE_INT: {
                const int val = *reinterpret_cast<const int*>(record->data + col.offset);
                v.set_int(val);
                break;
            }
            case TYPE_FLOAT: {
                const float val = *reinterpret_cast<const float*>(record->data + col.offset);
                v.set_float(val);
                break;
            }
            case TYPE_STRING: {
                std::string val(record->data + col.offset, col.len);
                v.set_str(val);
                break;
            }
            default:
                throw InternalError("Unexpected data type");
        }
        v.init_raw(col.len);
        return v;
    }
    
    // 设置列元数据
    void setupColumns(const std::vector<TabCol>& sel_cols, 
                      const std::vector<TabCol>& group_bys, 
                      const std::vector<Condition>& having_conds) {
        // 处理选择列
        for (size_t i = 0; i < sel_cols.size(); ++i) {
            const auto& sel_col = sel_cols[i];
            has_group_col_ |= agg_types_[i] == NO_AGG; // 在构造查询执行树时，已经保证没有出现在group by中的非聚合列不会出现在SELECT中
            
            // COUNT(*)特殊处理
            if (agg_types_[i] == AGG_COUNT && sel_col.tab_name.empty() && sel_col.col_name.empty()) {
                sel_cols_.emplace_back(); // 在sel_cols中原来是不包含这样的列的
                sel_cols_.back().type = TYPE_INT;
                sel_cols_.back().len = sizeof(int);
                sel_cols_.back().offset = sizeof(int);
            } else {
                sel_cols_.emplace_back(*get_col(cols_, sel_col));
                // COUNT输出总是整数
                if (agg_types_[i] == AGG_COUNT && sel_cols_.back().type != TYPE_INT) {
                    sel_cols_.back().type = TYPE_INT;
                    sel_cols_.back().len = sizeof(int);
                    // sel_cols_.back().offset = sizeof(int); 在Next中会重新设置
                }
            }
            len_ += sel_cols_.back().len;
        }
        
        // 处理HAVING列
        for (auto& having_cond : having_conds_) 
            setupHavingColumn(having_cond);
        
        // 处理GROUP BY列
        for (auto& group_by : group_bys) 
            group_bys_.emplace_back(*get_col(cols_, group_by));
    }
    
    // 设置HAVING条件的列元数据
    void setupHavingColumn(Condition& having_cond) {
        if (having_cond.agg_type == AGG_COUNT && 
            having_cond.lhs_col.tab_name.empty() && 
            having_cond.lhs_col.col_name.empty()) { // COUNT(*)
            having_cols_.emplace_back();
            having_cols_.back().type = TYPE_INT;
            having_cols_.back().len = sizeof(int);
            having_cols_.back().offset = sizeof(int);
        } else {
            having_cols_.emplace_back(*get_col(cols_, having_cond.lhs_col));
            // COUNT总是返回整数
            if (having_cond.agg_type == AGG_COUNT && having_cols_.back().type != TYPE_INT) {
                having_cols_.back().type = TYPE_INT;
                having_cols_.back().len = sizeof(int);
                having_cols_.back().offset = sizeof(int);
            }
        }
    }
    
    // 提取GROUP BY键
    std::vector<Value> extractGroupKeys(const RmRecord* record) {
        std::vector<Value> keys;
        keys.reserve(group_bys_.size());
        for (const auto& group_by : group_bys_) {
            keys.push_back(extractValue(record, group_by));
        }
        return keys;
    }
    
    // 提取聚合值
    std::vector<Value> extractSelectValues(const RmRecord* record) {
        std::vector<Value> values;
        values.reserve(agg_types_.size());
        
        for (size_t i = 0; i < agg_types_.size(); ++i) {
            switch (agg_types_[i]) {
                case AGG_COUNT: {
                    Value v;
                    v.set_int(1);
                    v.init_raw(sizeof(int));
                    values.push_back(v);
                    break;
                }
                case AGG_MAX: case AGG_MIN: case AGG_SUM: case AGG_AVG: {
                    values.push_back(extractValue(record, sel_cols_[i]));
                    break;
                }
                case NO_AGG:
                    values.emplace_back();
                    break;
                default:
                    throw InternalError("Unexpected aggregate type");
            }
        }
        return values;
    }
    
    // 提取HAVING值
    std::vector<Value> extractHavingValues(const RmRecord* record) {
        std::vector<Value> values;
        values.reserve(having_conds_.size());
        
        for (size_t i = 0; i < having_conds_.size(); ++i) {
            switch (having_conds_[i].agg_type) {
                case AGG_COUNT: {
                    Value v;
                    v.set_int(1);
                    v.init_raw(sizeof(int));
                    values.push_back(v);
                    break;
                }
                case AGG_MAX: case AGG_MIN: case AGG_SUM: case AGG_AVG: {
                    values.push_back(extractValue(record, having_cols_[i]));
                    break;
                }
                case NO_AGG:
                default:
                    throw InternalError("Unexpected aggregate type");
            }
        }
        return values;
    }
    
    // 检查分组是否满足HAVING条件
    bool meetsHavingConditions(const AggregateVal& agg_value) {
        for (size_t i = 0; i < having_conds_.size(); ++i) {
            if (!check_condition(agg_value.having_values[i], having_conds_[i].rhs_val, having_conds_[i])) 
                return false;
        }
        return true;
    }
    
    // 找到满足HAVING条件的下一个分组
    void findNextHavingGroup() {
        while (ht_iter_ != ht_.end()) {
            if (meetsHavingConditions(ht_iter_->second)) break;
            ++ht_iter_;
        }
    }
    
    // 处理空表COUNT聚合的特殊情况
    std::unique_ptr<RmRecord> createEmptyTableRecord() {
        auto record = std::make_unique<RmRecord>(len_);
        int offset = 0;
        
        for (size_t i = 0; i < agg_types_.size(); ++i) {
            if (agg_types_[i] == AGG_COUNT) {
                int zero = 0;
                memcpy(record->data + offset, &zero, sizeof(int));
                offset += sizeof(int);
            } else 
                throw InternalError("Unsupported aggregate null type");
        }
        
        return record;
    }

public:
    AggregateExecutor(std::unique_ptr<AbstractExecutor> prev, 
                      const std::vector<TabCol>& sel_cols,
                      std::vector<AggType> agg_types, 
                      const std::vector<TabCol>& group_bys,
                      std::vector<Condition> having_conds)
        : prev_(std::move(prev)), 
          agg_types_(std::move(agg_types)),
          having_conds_(std::move(having_conds)), 
          ht_(agg_types_, having_conds_) {
              
        cols_ = prev_->cols();
        
        // 不能直接在这里先设置AVG列的类型为float

        // 设置列元数据
        setupColumns(sel_cols, group_bys, having_conds_);
    }

    void beginTuple() override {
        // 清空哈希表，准备新一轮聚合
        ht_.clear();
        
        // 遍历所有输入记录，进行聚合
        for (prev_->beginTuple(); !prev_->is_end(); prev_->nextTuple()) {
            rm_record_ = prev_->Next();
            
            // 提取分组键、聚合值和HAVING值
            std::vector<Value> keys = extractGroupKeys(rm_record_.get()); // group by的键可能是包含多个列的复合键
            std::vector<Value> values = extractSelectValues(rm_record_.get()); // 获取用于聚合函数的字段的值
            std::vector<Value> having_values = extractHavingValues(rm_record_.get()); // 获取用于判断HAVING条件的字段的值
            
            // 插入或合并到哈希表，对于同样的key，values会被合并
            ht_.insert({std::move(keys)}, {std::move(values), std::move(having_values)});
        }

        // 处理AVG特殊情况
        for(size_t i = 0 ; i < agg_types_.size(); ++i) {
            if (agg_types_[i] != AGG_AVG) continue; // 只处理AVG
            // 对于AVG，先将所有值累加，最后除以计数器
            for(auto it = ht_.begin(); it != ht_.end(); ++it) {
                if (it->second.count == 0) continue; // 避免除以0
                it->second.values[i].value_cast(TYPE_FLOAT); // 保证其类型为浮点型
                it->second.values[i] = it->second.values[i] / it->second.count; // 计算平均值
                sel_cols_[i].type = TYPE_FLOAT; // 更新列类型为浮点型 // 后面会用到这个
            }
        }

        // 初始化迭代器
        ht_iter_ = ht_.begin();
        
        // 处理空表情况
        if (ht_iter_ == ht_.end()) {
            if (!group_bys_.empty() && has_group_col_) 
                return; // 空表+GROUP BY，输出空表
            is_empty_table_ = true; // 空表+COUNT，输出0
        } else 
            findNextHavingGroup(); // 找到第一个满足HAVING条件的分组
    }

    void nextTuple() override {
        if (is_empty_table_) {
            is_empty_table_ = false;
            return;
        }
        
        // 移动到下一个分组
        ++ht_iter_;
        findNextHavingGroup();
    }

    std::unique_ptr<RmRecord> Next() override {
        // 处理空表COUNT的特殊情况
        if (is_empty_table_) {
            return createEmptyTableRecord();
        }
        
        // 创建结果记录
        auto record = std::make_unique<RmRecord>(len_);
        int offset = 0;
        
        // 添加GROUP BY列的值
        if (has_group_col_) {
            for (size_t i = 0; i < group_bys_.size(); ++i) {
                const auto& key = ht_iter_->first.grouping_key[i];
                memcpy(record->data + offset, key.raw->data, group_bys_[i].len);
                offset += group_bys_[i].len;
            }
        }
        
        // 添加聚合列的值
        for (size_t i = 0; i < agg_types_.size(); ++i) {
            if (agg_types_[i] == NO_AGG) continue;
            
            const auto& value = ht_iter_->second.values[i];
            int len = agg_types_[i] == AGG_AVG ? sizeof(float) : sel_cols_[i].len; // AVG输出为float
            if (agg_types_[i] == AGG_COUNT) {
                memcpy(record->data + offset, &value.int_val, len);
            } else {
                memcpy(record->data + offset, value.raw->data, len);
            } 

            offset += len; // 其他聚合类型按原长度输出
        }
        
        return record;
    }

    Rid &rid() override { throw InternalError("AggregateExecutor does not have valid RID"); }

    bool is_end() const {
        if (is_empty_table_) return false; // 空表COUNT输出一次
        return ht_iter_ == ht_.end();
    }

    const std::vector<ColMeta> &cols() const override { return sel_cols_; }

    size_t tupleLen() const override { return len_; }

    std::string getType() { return "AggregateExecutor"; }
};

