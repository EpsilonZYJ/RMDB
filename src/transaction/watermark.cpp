/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "transaction/watermark.h"

/**
 * @brief 添加一个具有给定读时间戳的事务
 * @param read_ts 事务的读时间戳
 */
auto Watermark::AddTxn(timestamp_t read_ts) -> void {
  // 增加读时间戳的计数或添加新的时间戳
  current_reads_[read_ts]++;

  // 更新水印为当前所有活跃事务中最小的读时间戳
  // 第一个元素的键就是最小的读时间戳（std::map默认按键排序）
  if (!current_reads_.empty()) {
    watermark_ = current_reads_.begin()->first;
  }
}

/**
 * @brief 移除一个具有给定读时间戳的事务
 * @param read_ts 事务的读时间戳
 */
auto Watermark::RemoveTxn(timestamp_t read_ts) -> void {
  // 检查该读时间戳是否存在
  auto it = current_reads_.find(read_ts);
  if (it == current_reads_.end()) {
    return;
  }

  // 减少该读时间戳的计数
  it->second--;
  if (it->second == 0) {
    current_reads_.erase(it);
  }

  // 更新水印
  if (current_reads_.empty()) {
    // 如果没有活跃事务，将谁赢设置为最新的提交时间戳
    watermark_ = commit_ts_;
  } else {
    // 否则设置为当前活跃事务中最小的时间戳
    watermark_ = current_reads_.begin()->first;
  }
}

/**
 * @brief 更新提交时间戳
 * @param commit_ts 新的提交时间戳
 */
auto Watermark::UpdateCommitTs(timestamp_t commit_ts) -> void {
  commit_ts_ = commit_ts;
  // 如果没有活跃事务，同步更新水印
  if (current_reads_.empty()) {
    watermark_ = commit_ts_;
  }
}

/**
 * @brief 获取当前水印值
 * @return 当前水印值
 */
auto Watermark::GetWatermark() -> timestamp_t { return watermark_; }

