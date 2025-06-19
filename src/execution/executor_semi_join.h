#pragma once
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"
#include "check_condition.h"

class SemiJoinExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> left_;    // 左表执行器
    std::unique_ptr<AbstractExecutor> right_;   // 右表执行器
    size_t len_;                                // 结果记录长度(只包含左表)
    std::vector<ColMeta> cols_;                 // 结果记录的字段(只包含左表)
    std::vector<ColMeta> left_cols_;            // JOIN条件中左表字段
    std::vector<ColMeta> right_cols_;           // JOIN条件中右表字段

    std::vector<Condition> fed_conds_;          // JOIN条件
    bool isend;                                 // 是否遍历结束
    
    bool current_has_match_;  // 当前左表记录是否有匹配
    std::unique_ptr<RmRecord> current_matching_record_;
    std::set<uint64_t> processed_left_rids_;
    
    // 查找下一个匹配的记录
    void findNextMatch();
    // 添加到executor_semi_join.h的SemiJoinExecutor类内部
    // 通用比较函数
    bool compareValues(const char* left_value, const char* right_value, 
                    ColType type, CompOp op) {
        switch (type) {
            case TYPE_INT:
                return compareInts(*(int*)left_value, *(int*)right_value, op);
            case TYPE_FLOAT:
                return compareFloats(*(float*)left_value, *(float*)right_value, op);
            case TYPE_STRING:
            case TYPE_DATE:
                return compareStrings(left_value, right_value, op);
            default:
                std::cout << "DEBUG: 不支持的数据类型: " << type << std::endl;
                return false;
        }
    }
    // 整型
    bool compareInts(int left, int right, CompOp op) {
        bool result = false;
        switch (op) {
            case OP_EQ: result = (left == right); break;
            case OP_NE: result = (left != right); break;
            case OP_LT: result = (left < right); break;
            case OP_GT: result = (left > right); break;
            case OP_LE: result = (left <= right); break;
            case OP_GE: result = (left >= right); break;
            default: 
                std::cout << "DEBUG: 不支持的操作符: " << op << std::endl;
                return false;
        }
        
        std::cout << "DEBUG: 比较INT: " << left << " " << opToString(op) << " " 
                << right << " 结果: " << (result ? "匹配" : "不匹配") << std::endl;
        return result;
    }
    // 浮点数
    bool compareFloats(float left, float right, CompOp op) {
        bool result = false;
        switch (op) {
            case OP_EQ: result = (std::abs(left - right) < 0.000001f); break;
            case OP_NE: result = (std::abs(left - right) >= 0.000001f); break;
            case OP_LT: result = (left < right); break;
            case OP_GT: result = (left > right); break;
            case OP_LE: result = (left <= right); break;
            case OP_GE: result = (left >= right); break;
            default: 
                std::cout << "DEBUG: 不支持的操作符: " << op << std::endl;
                return false;
        }
        
        std::cout << "DEBUG: 比较FLOAT: " << left << " " << opToString(op) << " " 
                << right << " 结果: " << (result ? "匹配" : "不匹配") << std::endl;
        return result;
    }

    // 字符串比较
    bool compareStrings(const char* left_ptr, const char* right_ptr, CompOp op) {
        int len_left = *(int*)left_ptr;
        int len_right = *(int*)right_ptr;
        const char* str_left = left_ptr + sizeof(int);
        const char* str_right = right_ptr + sizeof(int);
        
        int cmp_result = strncmp(str_left, str_right, std::min(len_left, len_right));
        bool result = false;
        
        switch (op) {
            case OP_EQ: 
                result = (len_left == len_right && cmp_result == 0); break;
            case OP_NE: 
                result = (len_left != len_right || cmp_result != 0); break;
            case OP_LT: 
                result = (cmp_result < 0 || (cmp_result == 0 && len_left < len_right)); break;
            case OP_GT: 
                result = (cmp_result > 0 || (cmp_result == 0 && len_left > len_right)); break;
            case OP_LE: 
            result = (cmp_result < 0 || (cmp_result == 0 && len_left <= len_right));  break;
            case OP_GE: 
                result = (cmp_result > 0 || (cmp_result == 0 && len_left >= len_right));  break;
            default: 
                std::cout << "DEBUG: 不支持的操作符: " << op << std::endl;
                return false;
        }
        
        std::cout << "DEBUG: 比较STRING: 结果: " << (result ? "匹配" : "不匹配") << std::endl;
        return result;
    }

    // 操作符转字符串，用于调试输出
    std::string opToString(CompOp op) {
        switch (op) {
            case OP_EQ: return "==";
            case OP_NE: return "<>";
            case OP_LT: return "<";
            case OP_GT: return ">";
            case OP_LE: return "<=";
            case OP_GE: return ">=";
            default: return "?";
        }
    }


   public:
    SemiJoinExecutor(std::unique_ptr<AbstractExecutor> left, std::unique_ptr<AbstractExecutor> right, 
                      std::vector<Condition> conds) {
        left_ = std::move(left);
        right_ = std::move(right);
        
        // SEMI JOIN只需要左表的长度和列
        len_ = left_->tupleLen();
        cols_ = left_->cols();
        
        isend = false;
        fed_conds_ = std::move(conds);
        std::cout << "DEBUG: SemiJoinExecutor左表列: ";
        for (const auto& col : left_->cols()) {
            std::cout << col.tab_name << "." << col.name << " ";
        }
        std::cout << std::endl;
        std::cout << "DEBUG: SemiJoinExecutor右表列: ";
        for (const auto& col : right_->cols()) {
            std::cout << col.tab_name << "." << col.name << " ";
        }
        // 准备JOIN条件中的列
        for(auto &cond: fed_conds_) {
            std::cout<< "DEBUG: 处理JOIN条件: " << cond.lhs_col.col_name<<std::endl;
            left_cols_.push_back(*get_col(left_->cols(), cond.lhs_col));
            right_cols_.push_back(*get_col(right_->cols(), cond.rhs_col));
        }
           // 确认只使用左表列
            std::cout << "DEBUG: 半连接执行器初始化，保留列数: " << cols_.size() 
            << "，总长度: " << len_ << std::endl;
    }

    void beginTuple() override;
    void nextTuple() override;
    bool is_end() const override { return isend; }
    std::unique_ptr<RmRecord> Next(){ 
        if (isend || !current_matching_record_) 
        return nullptr;
    
    // 直接返回已缓存的匹配记录
    return std::make_unique<RmRecord>(*current_matching_record_);
        }
    const std::vector<ColMeta> &cols() const override { return cols_; }
    size_t tupleLen() const override { return len_; }
    Rid &rid() override { return _abstract_rid; }
};