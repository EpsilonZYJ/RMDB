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
<<<<<<< rmdb/src/record/rm_scan.cpp
    // if(file_handle_->file_hdr_.num_pages == 1){
    //     rid_={1,-1};
    //     return;
    // }//只有一个文件头
    // int page_ptr = 1;
    // RmPageHandle page_handle = file_handle_->fetch_page_handle(1);
    // while (page_ptr < file_handle_->file_hdr_.num_pages - 1&&page_handle.page_hdr->num_records == 0){
    //     file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
    //     page_handle = file_handle_->fetch_page_handle(++page_ptr);
    // }//找到第一个不为空的页面
    // int first_record = Bitmap::first_bit(1, page_handle.bitmap, file_handle_->file_hdr_.num_records_per_page);
    // if(file_handle_->file_hdr_.num_records_per_page==first_record){
    //     rid_.page_no = page_ptr;
    // }//无有效记录
    // else rid_ = {page_ptr, first_record};
    // file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
    rid_={1,-1};
    int start_page=rid_.page_no;
    int start_slot=rid_.slot_no;
	//从第一页的-1号位开始遍历
    for(int i = start_page; i < file_handle_->file_hdr_.num_pages; i++){
        auto page_handle = file_handle_->fetch_page_handle(i);
		if (page_handle.page_hdr->num_records != 0) {
			rid_ = {i, Bitmap::next_bit(true, page_handle.bitmap, page_handle.file_hdr->num_records_per_page, -1)};
			file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
			return;
		}
    }
    rid_ = {file_handle_->file_hdr_.num_pages, -1};
    return;
=======
    if(file_handle_->file_hdr_.num_pages == 1){
        rid_.page_no = file_handle_->file_hdr_.num_pages; // 超出页面范围表示结束
        rid_.slot_no = -1;
        return;
    }
    
    int page_ptr = 1;
    RmPageHandle page_handle = file_handle_->fetch_page_handle(1);
    
    // 找第一个非空页面
    while (page_ptr < file_handle_->file_hdr_.num_pages && 
           page_handle.page_hdr->num_records == 0){
        file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
        page_ptr++;
        
        // 检查是否已经到了表末尾
        if (page_ptr >= file_handle_->file_hdr_.num_pages) {
            rid_.page_no = file_handle_->file_hdr_.num_pages;  // 表示表已空
            rid_.slot_no = -1;
            return;
        }
        
        page_handle = file_handle_->fetch_page_handle(page_ptr);
    }
    
    // 找到页面的第一个有效记录
    int first_record = Bitmap::first_bit(1, page_handle.bitmap, 
                        file_handle_->file_hdr_.num_records_per_page);
    
    if(first_record >= file_handle_->file_hdr_.num_records_per_page){
        // 位图中找不到有效记录，可能位图与记录数不一致
        rid_.page_no = file_handle_->file_hdr_.num_pages;  // 标记为结束
        rid_.slot_no = -1;
    } else {
        // 找到有效记录
        rid_ = {page_ptr, first_record};
    }
    
    file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
>>>>>>> rmdb/src/record/rm_scan.cpp
}

/**
 * @brief 找到文件中下一个存放了记录的位置
 */
void RmScan::next() {
    // Todo:
    // 找到文件中下一个存放了记录的非空闲位置，用rid_来指向这个位置
    int Max= file_handle_->file_hdr_.num_records_per_page,slot_no;
    RmPageHandle page_handle = file_handle_->fetch_page_handle(rid_.page_no);
    while(1){
        slot_no = Bitmap::next_bit(1, page_handle.bitmap, Max, rid_.slot_no);
        if(slot_no < Max){
            rid_.slot_no=slot_no;
            file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
            break;
        }//找到了非空闲位置
        file_handle_->buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
        rid_.page_no++;
        if(is_end()){
            rid_.slot_no=-1;
            break;
        }//到文件末尾了
        page_handle = file_handle_->fetch_page_handle(rid_.page_no);
        rid_.slot_no = -1;//继续在下一页寻找
    }
}

/**
 * @brief ​ 判断是否到达文件末尾
 */
bool RmScan::is_end() const {
    // Todo: 修改返回值
    return rid_.page_no >= file_handle_->file_hdr_.num_pages;
}

/**
 * @brief RmScan内部存放的rid
 */
Rid RmScan::rid() const {
    return rid_;
}