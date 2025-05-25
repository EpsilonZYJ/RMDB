#pragma once

#include "common/common.h"
#include <system/sm.h>
bool check_condition(const RmRecord& record, TabMeta& tab_, const Condition& cond);

bool check_condition(const RmRecord& record, TabMeta& tab_, const std::vector<Condition>& conds);