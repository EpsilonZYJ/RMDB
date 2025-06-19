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

#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"

enum class BoundType { NO_BOUND, EQUAL, UPPER, LOWER };

struct Bound {
    BoundType bound_type;
    Value bound_value;
    int equal_index; // 如果是等值条件，则equal_index为该字段在conds_中的索引，否则为-1
    Bound() : bound_type(BoundType::NO_BOUND), 
              equal_index(-1) {}
};

class IndexScanExecutor : public AbstractExecutor {
   private:
    std::string tab_name_;                      // 表名称
    TabMeta tab_;                               // 表的元数据
    std::vector<Condition> conds_;              // 扫描条件
    RmFileHandle *fh_;                          // 表的数据文件句柄
    std::vector<ColMeta> cols_;                 // 需要读取的字段
    size_t len_;                                // 选取出来的一条记录的长度
    std::vector<Condition> fed_conds_;          // 扫描条件，和conds_字段相同

    std::vector<std::string> index_col_names_;  // index scan涉及到的索引包含的字段
    IndexMeta index_meta_;                      // index scan涉及到的索引元数据

    Rid rid_;
    std::unique_ptr<RecScan> scan_;

    SmManager *sm_manager_;                 

    std::vector<Bound> bounds_; // 存储每个条件对应的边界信息，长度应该和 conds_.size()相同

    /* 查找下一个符合条件的记录 */
    bool find_next_valid_rec() {
        for (; !scan_->is_end(); scan_->next()) {
            rid_ = scan_->rid();
            RmRecord rec = *fh_->get_record(rid_, context_);
            if (check_condition(rec, tab_, fed_conds_)) return true;
        }

        return false;
    }

    void get_lower_key(char* key) {
        int offset = 0;
        for (size_t i = 0; i < index_meta_.col_num; i++) {
            if (bounds_[i].bound_type == BoundType::LOWER || bounds_[i].bound_type == BoundType::EQUAL) {
                if(!bounds_[i].bound_value.raw) 
                    bounds_[i].bound_value.init_raw(index_meta_.cols[i].len); // 确保边界值的raw数据已初始化
                memcpy(key + offset, bounds_[i].bound_value.raw->data, index_meta_.cols[i].len);
            }
            else { // 如果没有下界，则设置为最小值。这里假设最小值为各类型的最小值
                switch (index_meta_.cols[i].type) {
                    case TYPE_INT:
                        *(int*)(key + offset) = std::numeric_limits<int>::min();
                        break;
                    case TYPE_FLOAT:
                        *(float*)(key + offset) = std::numeric_limits<float>::min();
                        break;
                    case TYPE_STRING:
                    case TYPE_DATE:
                        memset(key + offset, 0, index_meta_.cols[i].len);
                        break;
                    default:
                        break;
                }
            }
            offset += index_meta_.cols[i].len;
        }
    }


    void get_upper_key(char* key) {
        int offset = 0;
        for (size_t i = 0; i < index_meta_.col_num; i++) {
            if (bounds_[i].bound_type == BoundType::UPPER || bounds_[i].bound_type == BoundType::EQUAL) {
                if(!bounds_[i].bound_value.raw) 
                    bounds_[i].bound_value.init_raw(index_meta_.cols[i].len); // 确保边界值的raw数据已初始化
                memcpy(key + offset, bounds_[i].bound_value.raw->data, index_meta_.cols[i].len);
            }
            else { // 如果没有下界，则设置为最小值。这里假设最小值为各类型的最小值
                switch (index_meta_.cols[i].type) {
                    case TYPE_INT:
                        *(int*)(key + offset) = std::numeric_limits<int>::max();
                        break;
                    case TYPE_FLOAT:
                        *(float*)(key + offset) = std::numeric_limits<float>::max();
                        break;
                    case TYPE_STRING:
                    case TYPE_DATE:
                        memset(key + offset, 0xFF, index_meta_.cols[i].len);
                        break;
                    default:
                        break;
                }
            }
            offset += index_meta_.cols[i].len;
        }
    }

   public:
    IndexScanExecutor(SmManager *sm_manager, std::string tab_name, std::vector<Condition> conds, std::vector<std::string> index_col_names,
                    Context *context) {
        std::cout << "DEBUG: 使用索引扫描" << std::endl;

        sm_manager_ = sm_manager;
        context_ = context;
        tab_name_ = std::move(tab_name);
        tab_ = sm_manager_->db_.get_table(tab_name_);
        conds_ = std::move(conds);
        // index_no_ = index_no;
        index_col_names_ = index_col_names; 
        index_meta_ = *(tab_.get_index_meta(index_col_names_));
        fh_ = sm_manager_->fhs_.at(tab_name_).get();
        cols_ = tab_.cols;
        len_ = cols_.back().offset + cols_.back().len;
        std::map<CompOp, CompOp> swap_op = {
            {OP_EQ, OP_EQ}, {OP_NE, OP_NE}, {OP_LT, OP_GT}, {OP_GT, OP_LT}, {OP_LE, OP_GE}, {OP_GE, OP_LE},
        }; // 比较运算符的对应（比如 大于对小于）

        
        std::unordered_map<std::string, int> col_name_to_index;
        for (int i = 0; i < (int)index_meta_.col_num; i++) 
            col_name_to_index[index_meta_.cols[i].name] = i;
        bounds_.resize(index_meta_.col_num);
        // 保证条件中的左侧列在当前表上 // TODO 如果有一侧不在当前表，怎样index scan
        for (size_t i = 0; i < conds_.size(); i++) {
            auto &cond = conds_[i];
            if (cond.lhs_col.tab_name != tab_name_) {
                // lhs is on other table, now rhs must be on this table
                assert(!cond.is_rhs_val && cond.rhs_col.tab_name == tab_name_);
                // swap lhs and rhs
                std::swap(cond.lhs_col, cond.rhs_col);
                cond.op = swap_op.at(cond.op);
            }

            if(!cond.is_rhs_val || col_name_to_index.count(cond.lhs_col.col_name) == 0) continue;

            // 初始化边界信息，便于构建索引 // TODO best practice: 将这一部分从构造函数的逻辑中抽离
            Bound& bound = bounds_[col_name_to_index[cond.lhs_col.col_name]];
            switch (cond.op) {
                case OP_GT: case OP_GE:
                    bound.bound_type = BoundType::LOWER;
                    break;
                case OP_LE: case OP_LT:
                    bound.bound_type = BoundType::UPPER;
                    break;
                case OP_EQ:
                    bound.bound_type = BoundType::EQUAL;
                    bound.equal_index = i; // 记录等值条件在 **conds_** 中的索引
                    break;
                default:
                    break;
                
            }
            auto col_meta = tab_.get_col(cond.lhs_col.col_name);
            Value value = cond.rhs_val; // 拷贝构造避免修改原值
            if(col_meta->type != value.type) 
                value.value_cast(col_meta->type); // 确保新值与字段类型匹配
            bound.bound_value = value; // 设置边界值
        }

        // 计算连续的等值条件数量，从而可以在索引扫描时优化（即不需要判断这一部分条件）
        std::vector<int> not_ctig_eq(conds_.size(), 0); // 记录哪些条件不是连续的等值条件
        for (size_t i = 0; i < (size_t)index_meta_.col_num; i++) {
            if(bounds_[i].bound_type == BoundType::EQUAL) {
                assert(bounds_[i].equal_index >= 0 && bounds_[i].equal_index < (int)conds_.size());
                not_ctig_eq[bounds_[i].equal_index] = 1;
            } else break;
            
        }
        // 将 非 连续等值条件 放入fed_conds_
        for (size_t i = 0; i < conds_.size(); i++) {
            if(not_ctig_eq[i] == 0) fed_conds_.push_back(conds_[i]);
        }

    }

    void beginTuple() override {
        IxIndexHandle* ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index_col_names_)).get();

        std::unique_ptr<char>
            lower_key(new char[index_meta_.col_tot_len]),
            upper_key(new char[index_meta_.col_tot_len]);
        get_lower_key(lower_key.get());
        get_upper_key(upper_key.get());

        Iid lower = ih->lower_bound(lower_key.get());
        Iid upper = ih->upper_bound(upper_key.get());

        scan_ = std::make_unique<IxScan>(ih, lower, upper, sm_manager_->get_bpm());

        // 遍历获取第一个元素
        find_next_valid_rec();
    }

    void nextTuple() override {
        scan_->next();
        find_next_valid_rec();
    }

    std::unique_ptr<RmRecord> Next() override {
        return fh_->get_record(rid_, context_);
    }

    Rid &rid() override { return rid_; }

    bool is_end() const override { return scan_->is_end(); }

    size_t tupleLen() const override { return len_; }

    const std::vector<ColMeta> &cols() const override { return cols_; }
};