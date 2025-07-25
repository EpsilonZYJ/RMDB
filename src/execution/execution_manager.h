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

#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "execution_defs.h"
#include "record/rm.h"
#include "system/sm.h"
#include "common/context.h"
#include "common/common.h"
#include "optimizer/plan.h"
#include "executor_abstract.h"
#include "transaction/transaction_manager.h"
#include "optimizer/planner.h"

class Planner;

class QlManager {
   private:
    SmManager *sm_manager_;
    TransactionManager *txn_mgr_;
    Planner *planner_;
    LogManager *log_manager_;
    BufferPoolManager *buffer_pool_manager_;
    LockManager *lock_manager_;
   public:
   QlManager(LockManager *lock_manager,SmManager *sm_manager, TransactionManager *txn_mgr, Planner *planner,
    LogManager *log_manager=nullptr, BufferPoolManager *buffer_pool_manager=nullptr) 
    : sm_manager_(sm_manager), txn_mgr_(txn_mgr), planner_(planner),
    log_manager_(log_manager), buffer_pool_manager_(buffer_pool_manager),lock_manager_(lock_manager) {}

    void run_mutli_query(std::shared_ptr<Plan> plan, Context *context,txn_id_t *txn_id);
    void run_cmd_utility(std::shared_ptr<Plan> plan, txn_id_t *txn_id, Context *context);
    void select_from(std::unique_ptr<AbstractExecutor> executorTreeRoot, std::vector<TabCol> sel_cols,
                        Context *context);

    void run_dml(std::unique_ptr<AbstractExecutor> exec);
    void run_explain(std::shared_ptr<Plan> plan, Context *context);
    void load_data(const std::string& file_name, const std::string& table_name, Transaction* txn);
};
