#include "executor_filter.h"
#include "execution_manager.h"
#include "optimizer/plan.h"
#include "check_condition.h"
#include "executor_seq_scan.h"      
#include "executor_index_scan.h"   
#include "executor_nestedloop_join.h" 
#include "executor_projection.h"     

void FilterExecutor::beginTuple() {
    subplan_executor_->beginTuple();
    executed_ = false;
}

void FilterExecutor::nextTuple() {
    subplan_executor_->nextTuple();
}

// std::unique_ptr<RmRecord> FilterExecutor::Next() {
//     while (!subplan_executor_->is_end()) {
//         auto rec = subplan_executor_->Next();
//         if (!rec) continue;
        
//         bool pass_filter = true;
//         for (const auto &cond : conds_) {
//             if (!check_condition(rec.get(), &cond, cols(), nullptr, nullptr)) {
//                 pass_filter = false;
//                 break;
//             }
//         }
        
//         if (pass_filter) {
//             return rec;
//         }
//     }
//     return nullptr;
// }

std::unique_ptr<RmRecord> FilterExecutor::Next() {
    try {
        while (!subplan_executor_->is_end()) {
            auto rec = subplan_executor_->Next();
            if (!rec) continue;
            
            bool pass_filter = true;
            // 防御性代码：确保有列
            auto all_cols = cols();
            if (all_cols.empty()) {
                std::cerr << "警告: FilterExecutor::cols() 返回空列表" << std::endl;
            }
            
            for (const auto &cond : conds_) {
                try {
                    // 检查列引用是否有效
                    bool found_lhs = false, found_rhs = false;
                    for (const auto& col : all_cols) {
                        if (col.tab_name == cond.lhs_col.tab_name && 
                            col.name == cond.lhs_col.col_name) {
                            found_lhs = true;
                        }
                        if (!cond.is_rhs_val && 
                            col.tab_name == cond.rhs_col.tab_name && 
                            col.name == cond.rhs_col.col_name) {
                            found_rhs = true;
                        }
                    }
                    
                    if (!found_lhs || (!cond.is_rhs_val && !found_rhs)) {
                        std::cerr << "警告: 找不到条件中的列: " 
                                  << cond.lhs_col.tab_name << "." << cond.lhs_col.col_name;
                        if (!cond.is_rhs_val) {
                            std::cerr << " 或 " << cond.rhs_col.tab_name << "." << cond.rhs_col.col_name;
                        }
                        std::cerr << std::endl;
                        pass_filter = false;
                        break;
                    }
                    
                    // 验证通过后进行条件检查
                    if (!check_condition(rec.get(), &cond, all_cols, nullptr, nullptr)) {
                        pass_filter = false;
                        break;
                    }
                } catch (...) {
                    std::cerr << "警告: 条件检查异常" << std::endl;
                    pass_filter = false;
                    break;
                }
            }
            
            if (pass_filter) {
                return rec;
            }
        }
        return nullptr;
    } catch (const std::exception& e) {
        std::cerr << "错误: FilterExecutor::Next 异常: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cerr << "错误: FilterExecutor::Next 未知异常" << std::endl;
        return nullptr;
    }
}

bool FilterExecutor::is_end() const {
    return subplan_executor_->is_end();
}

const std::vector<ColMeta> &FilterExecutor::cols() const {
    return subplan_executor_->cols();
}

Rid &FilterExecutor::rid() {
    return subplan_executor_->rid();
}


// std::unique_ptr<AbstractExecutor> create_executor_for_plan(std::shared_ptr<Plan> plan, Context* context, SmManager* sm_manager) {
//     // 根据计划类型创建相应的执行器
//     if (auto scan_plan = std::dynamic_pointer_cast<ScanPlan>(plan)) {
//         if (scan_plan->tag == T_SeqScan) {
//             return std::make_unique<SeqScanExecutor>(sm_manager,scan_plan->tab_name_, scan_plan->conds_, context);
//         } else if (scan_plan->tag == T_IndexScan) {
//             return std::make_unique<IndexScanExecutor>(sm_manager,scan_plan->tab_name_,scan_plan->conds_,scan_plan->index_col_names_,context);
//         }
//     }
//     else if (auto join_plan = std::dynamic_pointer_cast<JoinPlan>(plan)) {
//         // 添加JoinPlan处理逻辑
//         auto left = create_executor_for_plan(join_plan->left_, context, sm_manager);
//         auto right = create_executor_for_plan(join_plan->right_, context, sm_manager);
//         return std::make_unique<NestedLoopJoinExecutor>(
//             std::move(left), 
//             std::move(right), 
//             join_plan->conds_
//         );
//     }
//      else if (auto proj_plan = std::dynamic_pointer_cast<ProjectionPlan>(plan)) {
//         auto child = create_executor_for_plan(proj_plan->subplan_, context,sm_manager);
//         return std::make_unique<ProjectionExecutor>(
//             std::move(child),
//             proj_plan->sel_cols_
//         );
//     } else if (auto filter_plan = std::dynamic_pointer_cast<FilterPlan>(plan)) {
//         auto child = create_executor_for_plan(filter_plan->subplan_, context, sm_manager);
//         return std::make_unique<FilterExecutor>(filter_plan, std::move(child));
//     }
//     throw std::runtime_error("未知的计划类型");
// }