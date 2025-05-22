/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "rm_scan.h"
#include "rm_file_handle.h"

/**
 * @brief 初始化file_handle和rid
 * @param file_handle
 */
RmScan::RmScan(const RmFileHandle *file_handle) : file_handle_(file_handle) {
    // Todo:
    // 初始化file_handle和rid（指向第一个存放了记录的位置）
    if(file_handle_->file_hdr_.num_pages == 1){
        rid_={1,-1};
        return;
    }//只有一个文件头
    int i = 1;
    RmPageHandle page_handle = file_handle_->fetch_page_handle(1);
    while (i < file_handle_->file_hdr_.num_pages - 1&&page_handle.page_hdr->num_records == 0)
    {
        file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
        page_handle = file_handle_->fetch_page_handle(++i);
    }//找到第一个不为空的页面
    int first_record = Bitmap::first_bit(1, page_handle.bitmap, file_handle_->file_hdr_.num_records_per_page);
    if(file_handle_->file_hdr_.num_records_per_page==first_record){
        rid_ = {i, -1};
    }//无有效记录
    else rid_ = {i, first_record};
    file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
}

/**
 * @brief 找到文件中下一个存放了记录的位置
 */
void RmScan::next() {
    // Todo:
    // 找到文件中下一个存放了记录的非空闲位置，用rid_来指向这个位置
    if (rid_.page_no >= file_handle_->file_hdr_.num_pages) {
        return;//到文件末尾了
    }
    RmPageHandle page_handle = file_handle_->fetch_page_handle(rid_.page_no);
    int next_slot = Bitmap::next_bit(1, page_handle.bitmap, rid_.slot_no + 1, file_handle_->file_hdr_.num_records_per_page);//下一个空闲槽
    if (next_slot < file_handle_->file_hdr_.num_records_per_page) {
        rid_.slot_no = next_slot;
        file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
        return;
    }//找到了有效记录
    file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
    rid_.page_no++;
    rid_.slot_no = -1;//到下一页
    while (rid_.page_no < file_handle_->file_hdr_.num_pages) {
        page_handle = file_handle_->fetch_page_handle(rid_.page_no);
        next_slot = Bitmap::first_bit(1, page_handle.bitmap,file_handle_->file_hdr_.num_records_per_page);    
        file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
        if (next_slot < file_handle_->file_hdr_.num_records_per_page) {
            rid_.slot_no = next_slot;
            return;
        }  
        rid_.page_no++;
    }//重复查找
    rid_.page_no = file_handle_->file_hdr_.num_pages;
}

/**
 * @brief ​ 判断是否到达文件末尾
 */
bool RmScan::is_end() const {
    // Todo: 修改返回值
    return rid_.page_no >= file_handle_->file_hdr_.num_pages || (rid_.slot_no == -1 && rid_.page_no > 0);
}

/**
 * @brief RmScan内部存放的rid
 */
Rid RmScan::rid() const {
    return rid_;
}