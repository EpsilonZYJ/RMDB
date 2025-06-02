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

#include <sstream>
#include "execution/executor_abstract.h"
#include "optimizer/plan.h"
#include "optimizer/planner.h"

class ExplainExecutor : public AbstractExecutor {
private:
    ExplainPlan *plan_;
    bool executed_;
    std::string result_; // 存储EXPLAIN的结果
    
public: 
    ExplainExecutor(ExplainPlan *plan, Context *context)
    : AbstractExecutor(), plan_(plan), executed_(false){
    } 
    
    void beginTuple() override;
    
    void nextTuple() override;
    
    std::unique_ptr<RmRecord> Next() override;
    
    void endTuple() {}
    bool is_end() const override;
    const std::vector<ColMeta> &cols() const override;
    Rid &rid() override;
private:
    // 生成EXPLAIN输出
    void generate_explain_output();
    std::vector<ColMeta> explain_cols_;
};