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
    size_t len_;                                // 结果记录长度(只包含左表长度)
    std::vector<ColMeta> cols_;                 // 结果记录的字段(只包含左表字段)
    std::vector<ColMeta> left_cols_;            // JOIN条件中左表字段
    std::vector<ColMeta> right_cols_;           // JOIN条件中右表字段

    std::vector<Condition> fed_conds_;          // JOIN条件
    bool isend;                                 // 是否遍历结束
    
    // 记录已匹配的左表记录，避免重复
    std::set<uint64_t> matched_left_rids_;
    
    // 查找下一个匹配的记录
    void findNextMatch();

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
        
        // 准备JOIN条件中的列
        for(auto &cond: fed_conds_) {
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
    std::unique_ptr<RmRecord> Next(){return left_->Next();}
    const std::vector<ColMeta> &cols() const override { return cols_; }
    size_t tupleLen() const override { return len_; }
    Rid &rid() override { return _abstract_rid; }
};