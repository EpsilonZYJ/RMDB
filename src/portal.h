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

#include <cerrno>
#include <cstring>
#include <string>
#include "optimizer/plan.h"
#include "execution/executor_abstract.h"
#include "execution/executor_nestedloop_join.h"
#include "execution/executor_projection.h"
#include "execution/executor_seq_scan.h"
#include "execution/executor_index_scan.h"
#include "execution/executor_update.h"
#include "execution/executor_insert.h"
#include "execution/executor_delete.h"
#include "execution/execution_sort.h"
#include "execution/executor_aggregate.h"
#include "execution/executor_limit.h"
#include "common/common.h"
#include "execution/executor_semi_join.h"

typedef enum portalTag{
    PORTAL_Invalid_Query = 0,
    PORTAL_ONE_SELECT,
    PORTAL_DML_WITHOUT_SELECT,
    PORTAL_MULTI_QUERY,
    PORTAL_CMD_UTILITY,
    PORTAL_EXPLAIN
} portalTag;


struct PortalStmt {
    portalTag tag;
    
    std::vector<TabCol> sel_cols;
    std::unique_ptr<AbstractExecutor> root;
    std::shared_ptr<Plan> plan;
    
    PortalStmt(portalTag tag_, std::vector<TabCol> sel_cols_, std::unique_ptr<AbstractExecutor> root_, std::shared_ptr<Plan> plan_) :
            tag(tag_), sel_cols(std::move(sel_cols_)), root(std::move(root_)), plan(std::move(plan_)) {}
};

class Portal
{
   private:
    SmManager *sm_manager_;
    

   public:
    Portal(SmManager *sm_manager) : sm_manager_(sm_manager){}
    ~Portal(){}

    // 将查询执行计划转换成对应的算子树
    std::shared_ptr<PortalStmt> start(std::shared_ptr<Plan> plan, Context *context)
    {   
        if (auto create_checkpoint = std::dynamic_pointer_cast<CreateCheckpointPlan>(plan)) {
            // 使用正确的构造函数创建 PortalStmt
            return std::make_shared<PortalStmt>(
                PORTAL_MULTI_QUERY,          // 使用命令工具类型
                std::vector<TabCol>(),       // 无选择列
                std::unique_ptr<AbstractExecutor>(),  // 无执行器
                plan                         // 检查点计划
            );
        }
        else if (auto x = std::dynamic_pointer_cast<ExplainPlan>(plan)) {
            // 创建一个特殊的 PortalStmt 来处理 EXPLAIN 命令
            return std::make_shared<PortalStmt>(PORTAL_EXPLAIN, std::vector<TabCol>(), std::unique_ptr<AbstractExecutor>(), plan);
        }
        // 这里可以将select进行拆分，例如：一个select，带有return的select等
        else if (auto x = std::dynamic_pointer_cast<OtherPlan>(plan)) {
            return std::make_shared<PortalStmt>(PORTAL_CMD_UTILITY, std::vector<TabCol>(), std::unique_ptr<AbstractExecutor>(),plan);
        } else if(auto x = std::dynamic_pointer_cast<SetKnobPlan>(plan)) {
            return std::make_shared<PortalStmt>(PORTAL_CMD_UTILITY, std::vector<TabCol>(), std::unique_ptr<AbstractExecutor>(), plan); 
        } else if (auto x = std::dynamic_pointer_cast<DDLPlan>(plan)) {
            return std::make_shared<PortalStmt>(PORTAL_MULTI_QUERY, std::vector<TabCol>(), std::unique_ptr<AbstractExecutor>(),plan);
        } else if (auto x = std::dynamic_pointer_cast<DMLPlan>(plan)) {
            switch(x->tag) {
                case T_select:
                {
                    std::shared_ptr<ProjectionPlan> p = std::dynamic_pointer_cast<ProjectionPlan>(x->subplan_);
                    std::unique_ptr<AbstractExecutor> root= convert_plan_executor(p, context);// TODO !!!

                    // 处理别名
                    std::vector<TabCol> cols_to_be_showed = p->sel_cols_;
                    for(auto &col : cols_to_be_showed) {
                        std::cout << "DEBUG::处理别名前：select_from sel_col.col_name: " << col.col_name << std::endl;
                    }
                    for (size_t i = 0; i < p->alias_.size(); i++){ 
                        if (!p->alias_[i].empty()) 
                            cols_to_be_showed[i].col_name = std::move(p->alias_[i]);
                        }
                    for(auto it:cols_to_be_showed) {
                        std::cout << "DEBUG:处理别名后： select_from sel_col.col_name: " << it.col_name << std::endl;
                    }
                    return std::make_shared<PortalStmt>(
                                PORTAL_ONE_SELECT, 
                                std::move(cols_to_be_showed), 
                                std::move(root), plan
                            );
                }
                    
                case T_Update:
                {
                    std::unique_ptr<AbstractExecutor> scan= convert_plan_executor(x->subplan_, context);
                    std::vector<Rid> rids;
                    for (scan->beginTuple(); !scan->is_end(); scan->nextTuple()) {
                        rids.push_back(scan->rid());
                    }
                    std::unique_ptr<AbstractExecutor> root =std::make_unique<UpdateExecutor>(sm_manager_, 
                                                            x->tab_name_, x->set_clauses_, x->conds_, rids, context);
                    return std::make_shared<PortalStmt>(PORTAL_DML_WITHOUT_SELECT, std::vector<TabCol>(), std::move(root), plan);
                }
                case T_Delete:
                {
                    std::unique_ptr<AbstractExecutor> scan= convert_plan_executor(x->subplan_, context);
                    std::vector<Rid> rids;
                    for (scan->beginTuple(); !scan->is_end(); scan->nextTuple()) {
                        rids.push_back(scan->rid());
                    }

                    std::unique_ptr<AbstractExecutor> root =
                        std::make_unique<DeleteExecutor>(sm_manager_, x->tab_name_, x->conds_, rids, context);

                    return std::make_shared<PortalStmt>(PORTAL_DML_WITHOUT_SELECT, std::vector<TabCol>(), std::move(root), plan);
                }

                case T_Insert:
                {
                    std::unique_ptr<AbstractExecutor> root =
                            std::make_unique<InsertExecutor>(sm_manager_, x->tab_name_, x->values_, context);
            
                    return std::make_shared<PortalStmt>(PORTAL_DML_WITHOUT_SELECT, std::vector<TabCol>(), std::move(root), plan);
                }


                default:
                    throw InternalError("Unexpected field type");
                    break;
            }
        } else {
            throw InternalError("Unexpected field type");
        }
        return nullptr;
    }

    // 遍历算子树并执行算子生成执行结果
    void run(std::shared_ptr<PortalStmt> portal, QlManager* ql, txn_id_t *txn_id, Context *context){
        if (context->txn_ != nullptr) {
            std::cout << "DEBUG: Portal::run - 事务ID: " << context->txn_->get_transaction_id() 
                      << ", 模式: " << (context->txn_->get_txn_mode() ? "显式" : "隐式") 
                      << ", 状态: " << static_cast<int>(context->txn_->get_state())
                      << std::endl;
        } else {
            std::cout << "DEBUG: Portal::run - 无活跃事务" << std::endl;
        }
        switch(portal->tag) {
            case PORTAL_ONE_SELECT:
            {
                for(auto &col : portal->sel_cols) {
                    std::cout << "DEBUG: run::select_from sel_col.col_name: " << col.col_name << std::endl;
                }
                ql->select_from(std::move(portal->root), std::move(portal->sel_cols), context);
                break;
            }

            case PORTAL_DML_WITHOUT_SELECT:
            {
                ql->run_dml(std::move(portal->root));
                break;
            }
            case PORTAL_MULTI_QUERY:
            {
                ql->run_mutli_query(portal->plan, context);
                break;
            }
            case PORTAL_CMD_UTILITY:
            {
                ql->run_cmd_utility(portal->plan, txn_id, context);
                break;
            }
            case PORTAL_EXPLAIN:
            {
            if (auto explain_plan = std::dynamic_pointer_cast<ExplainPlan>(portal->plan)) {
                // 创建 ExplainExecutor
                auto executor = explain_plan->get_executor(context);   
                // 定义输出列
                std::vector<TabCol> explain_cols = {TabCol{"", "EXPLAIN"}}; 
                // 使用 select_from 显示结果
                ql->select_from(std::move(executor), std::move(explain_cols), context);
            }
            break;
            }
            default:
            {
                throw InternalError("Unexpected field type");
            }
        }
        std::cout << "DEBUG: Portal::run - 执行完成" << std::endl;
    }

    // 清空资源
    void drop(){}


    std::unique_ptr<AbstractExecutor> convert_plan_executor(std::shared_ptr<Plan> plan, Context *context)
    {
        if (auto x = std::dynamic_pointer_cast<ExplainPlan>(plan)) {
            return x->get_executor(context);
        } else if(auto x = std::dynamic_pointer_cast<ProjectionPlan>(plan)){
            std::cout<<"DEBUG: convert_plan_executor ProjectionPlan" << std::endl;
            return std::make_unique<ProjectionExecutor>(convert_plan_executor(x->subplan_, context), 
                                                        x->sel_cols_);
            std::cout<<"DEBUG: convert_plan_executor ProjectionPlan 结束" << std::endl;
        } else if(auto x = std::dynamic_pointer_cast<ScanPlan>(plan)) {
            if(x->tag == T_SeqScan) {
                return std::make_unique<SeqScanExecutor>(sm_manager_, x->tab_name_, x->conds_, context);
            }
            else {
                return std::make_unique<IndexScanExecutor>(sm_manager_, x->tab_name_, x->conds_, x->index_col_names_, context);
            } 
        } else if(auto x = std::dynamic_pointer_cast<JoinPlan>(plan)) {
            std::unique_ptr<AbstractExecutor> left = convert_plan_executor(x->left_, context);
            std::unique_ptr<AbstractExecutor> right = convert_plan_executor(x->right_, context);
            // 根据JOIN类型创建不同的执行器
            if (x->type == SEMI_JOIN) {
                std::cout << "DEBUG: 创建半连接执行器SemiJoinExecutor" << std::endl;
                return std::make_unique<SemiJoinExecutor>(
                    std::move(left), 
                    std::move(right), 
                    std::move(x->conds_)
                );
            } else {
                // 默认使用嵌套循环连接
                return std::make_unique<NestedLoopJoinExecutor>(
                    std::move(left), 
                    std::move(right), 
                    std::move(x->conds_)
                );
            }
        } else if(auto x = std::dynamic_pointer_cast<SortPlan>(plan)) {
            return std::make_unique<SortExecutor>(convert_plan_executor(x->subplan_, context), 
                                            x->sel_col_, x->is_desc_);
        } else if (auto x = std::dynamic_pointer_cast<AggregatePlan>(plan)) {
            return std::make_unique<AggregateExecutor>(
                convert_plan_executor(x->subplan_, context),
                std::move(x->sel_cols_),
                std::move(x->agg_types_),
                std::move(x->group_bys_), 
                std::move(x->havings_));
        } else if (auto x = std::dynamic_pointer_cast<LimitPlan>(plan)) {
            return std::make_unique<LimitExecutor>(
                convert_plan_executor(x->subplan_, context), x->limit_num_);
        }
        return nullptr;
    }

};