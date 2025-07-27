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
#include "check_condition.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"

class UpdateExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;                       // 表的元和数据
    std::vector<Condition> conds_;      // update的条件(?)
    RmFileHandle *fh_;                  // 表的数据文件句柄
    std::vector<Rid> rids_;             // 各个记录的标识，表示update的位置(?)
    std::string tab_name_;              // 表名称
    std::vector<SetClause> set_clauses_;// 更新语句
    SmManager *sm_manager_;             // 系统管理器，负责元数据管理和DDL语句的执行
   public:
    UpdateExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<SetClause> set_clauses,
                   std::vector<Condition> conds, std::vector<Rid> rids, Context *context) {
        sm_manager_ = sm_manager;
        tab_name_ = tab_name;
        set_clauses_ = set_clauses;
        tab_ = sm_manager_->db_.get_table(tab_name);
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        conds_ = conds;
        rids_ = rids;
        context_ = context;
    }

    // 由于是update，Next()只调用一次（同insert，delete）
    std::unique_ptr<RmRecord> Next() override {
    for (auto& rid : rids_) {
        RmRecord old_record = *fh_->get_record(rid, context_);
        if (!check_condition(old_record, tab_, conds_)) continue;

        // 构造新记录
        RmRecord new_record(old_record);

        for (auto& set_clause : set_clauses_) {
            auto col_meta = tab_.get_col(set_clause.lhs.col_name);
            int offset = col_meta->offset;
            Value value;

            if (set_clause.is_expr && !set_clause.ref_col.col_name.empty()) {
                // 右侧为列或表达式
                auto ref_col_meta = tab_.get_col(set_clause.ref_col.col_name);
                int ref_offset = ref_col_meta->offset;
                Value orig_value;
                switch (ref_col_meta->type) {
                    case TYPE_INT: orig_value.set_int(*(int*)(old_record.data + ref_offset)); break;
                    case TYPE_FLOAT: orig_value.set_float(*(float*)(old_record.data + ref_offset)); break;
                    case TYPE_STRING: orig_value.set_str(std::string(old_record.data + ref_offset, ref_col_meta->len)); break;
                    default: throw InternalError("不支持的列类型");
                }
                Value rhs_value = set_clause.rhs;
                if (orig_value.type != rhs_value.type) rhs_value.value_cast(orig_value.type);

                // 计算表达式结果
                switch (set_clause.op_type) {
                    case '+':
                        if (orig_value.type == TYPE_INT) value.set_int(orig_value.int_val + rhs_value.int_val);
                        else if (orig_value.type == TYPE_FLOAT) value.set_float(orig_value.float_val + rhs_value.float_val);
                        break;
                    case '-':
                        if (orig_value.type == TYPE_INT) value.set_int(orig_value.int_val - rhs_value.int_val);
                        else if (orig_value.type == TYPE_FLOAT) value.set_float(orig_value.float_val - rhs_value.float_val);
                        break;
                    case '*':
                        if (orig_value.type == TYPE_INT) value.set_int(orig_value.int_val * rhs_value.int_val);
                        else if (orig_value.type == TYPE_FLOAT) value.set_float(orig_value.float_val * rhs_value.float_val);
                        break;
                    case '/':
                        if ((rhs_value.type == TYPE_INT && rhs_value.int_val == 0) ||
                            (rhs_value.type == TYPE_FLOAT && rhs_value.float_val == 0.0))
                            throw InternalError("除数不能为零");
                        if (orig_value.type == TYPE_INT) value.set_int(orig_value.int_val / rhs_value.int_val);
                        else if (orig_value.type == TYPE_FLOAT) value.set_float(orig_value.float_val / rhs_value.float_val);
                        break;
                    default: throw InternalError("不支持的运算符");
                }
            } else {
                // 右侧为值
                value = set_clause.rhs;
                if (value.type != col_meta->type) value.value_cast(col_meta->type);
            }

            value.init_raw(col_meta->len);
            memcpy(new_record.data + offset, value.raw->data, col_meta->len);
        }

        // 索引 key 构造
        auto **old_keys = new char *[tab_.indexes.size()];
        auto **new_keys = new char *[tab_.indexes.size()];
        auto **ihs = new IxIndexHandle *[tab_.indexes.size()];
        for (size_t i = 0; i < tab_.indexes.size(); ++i) {
            auto &index = tab_.indexes[i];
            auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();
            old_keys[i] = new char[index.col_tot_len];
            new_keys[i] = new char[index.col_tot_len];
            ihs[i] = ih;
            int offset = 0;
            for (size_t j = 0; j < index.col_num; ++j) {
                memcpy(old_keys[i] + offset, old_record.data + index.cols[j].offset, index.cols[j].len);
                memcpy(new_keys[i] + offset, new_record.data + index.cols[j].offset, index.cols[j].len);
                offset += index.cols[j].len;
            }
            // 用 memcmp 判断 key 是否变化
            if (memcmp(old_keys[i], new_keys[i], index.col_tot_len) != 0 && !ihs[i]->is_unique(new_keys[i], context_->txn_)) {
                for (int j = 0; j <= i; ++j) {
                    delete []old_keys[j];
                    delete []new_keys[j];
                }
                delete []old_keys;
                delete []new_keys;
                delete []ihs;
                throw IndexNotUniqueError(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols));
            }
        }

        // 写集保存 old_record 副本
        if (context_ && context_->txn_) {
            RmRecord record_copy(old_record.size);
            memcpy(record_copy.data, old_record.data, old_record.size);
            WriteRecord* write_record = new WriteRecord(WType::UPDATE_TUPLE, tab_name_, rid, record_copy);
            context_->txn_->append_write_record(write_record);
        }

        fh_->update_record(rid, new_record.data, context_);

        for (size_t i = 0; i < tab_.indexes.size(); ++i) {
            ihs[i]->delete_entry(old_keys[i], context_->txn_);
            ihs[i]->insert_entry(new_keys[i], rid, context_->txn_);
            delete []old_keys[i];
            delete []new_keys[i];
        }
        delete []old_keys;
        delete []new_keys;
        delete []ihs;
    }
    return nullptr;
}

    Rid &rid() override { return _abstract_rid; }
};