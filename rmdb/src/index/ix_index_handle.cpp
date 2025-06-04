/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "ix_index_handle.h"

#include "ix_scan.h"

/**
 * @brief 在当前node中查找第一个>=target的key_idx
 *
 * @return key_idx，范围为[0,num_key)，如果返回的key_idx=num_key，则表示target大于最后一个key
 * @note 返回key index（同时也是rid index），作为slot no
 */
int IxNodeHandle::lower_bound(const char *target) const {
    // Todo:
    // 查找当前节点中第一个大于等于target的key，并返回key的位置给上层
    // 提示: 可以采用多种查找方式，如顺序遍历、二分查找等；使用ix_compare()函数进行比较
    int left = 0, right = page_hdr->num_key;
    int mid;
    // 二分法查找
    while(left < right){
        mid = (left + right) >> 1;
        int cmp = ix_compare(get_key(mid), target, file_hdr->col_types_, file_hdr->col_lens_);
        if(cmp < 0){
            left = mid + 1;
        }
        else{
            right = mid;
        }
    }
    return left;
}

/**
 * @brief 在当前node中查找第一个>target的key_idx
 *
 * @return key_idx，范围为[1,num_key)，如果返回的key_idx=num_key，则表示target大于等于最后一个key
 * @note 注意此处的范围从1开始
 */
int IxNodeHandle::upper_bound(const char *target) const {
    // Todo:
    // 查找当前节点中第一个大于target的key，并返回key的位置给上层
    // 提示: 可以采用多种查找方式：顺序遍历、二分查找等；使用ix_compare()函数进行比较
    int left = 1, right = page_hdr->num_key;
    int mid;

    // 如果只有一个元素或为空结点
    if(page_hdr->num_key <= 1){
        // 如果为空节点
        if(page_hdr->num_key == 0)
            return 0;
        // 如果只有一个元素
        int cmp = ix_compare(get_key(0), target, file_hdr->col_types_, file_hdr->col_lens_);
        if(cmp > 0) // 唯一的元素大于目标值
            return 1;
        else
            return page_hdr->num_key;
    }

    // 二分查找
    while(left < right){
        mid = (left + right) >> 1;
        int cmp = ix_compare(get_key(mid), target, file_hdr->col_types_, file_hdr->col_lens_);
        if(cmp <= 0){
            left = mid + 1;
        }
        else{
            right = mid;
        }
    }

    // 确保返回值至少为1，除非节点为空
    if(left == 0 && page_hdr->num_key > 0){
        return 1;
    }

    if(left < page_hdr->num_key) {
        return left;
    }
    else{
        return page_hdr->num_key;
    }
}

/**
 * @brief 用于叶子结点根据key来查找该结点中的键值对
 * 值value作为传出参数，函数返回是否查找成功
 *
 * @param key 目标key
 * @param[out] value 传出参数，目标key对应的Rid
 * @return 目标key是否存在
 */
bool IxNodeHandle::leaf_lookup(const char *key, Rid **value) {
    // Todo:
    // 1. 在叶子节点中获取目标key所在位置
    // 2. 判断目标key是否存在
    // 3. 如果存在，获取key对应的Rid，并赋值给传出参数value
    // 提示：可以调用lower_bound()和get_rid()函数。
    int index = lower_bound(key);
    if(index == page_hdr->num_key) {
        // 如果index等于num_key，说明key大于所有的key
        return false;
    }
    else if(!memcmp(get_key(index), key, file_hdr->col_tot_len_)){
        *value = get_rid(index);
        return true;
    }
    else{
        return false;
    }
}

/**
 * 用于内部结点（非叶子节点）查找目标key所在的孩子结点（子树）
 * @param key 目标key
 * @return page_id_t 目标key所在的孩子节点（子树）的存储页面编号
 */
page_id_t IxNodeHandle::internal_lookup(const char *key) {
    // Todo:
    // 1. 查找当前非叶子节点中目标key所在孩子节点（子树）的位置
    // 2. 获取该孩子节点（子树）所在页面的编号
    // 3. 返回页面编号
    assert(!is_leaf_page()); // 确保当前结点是内部结点
    int index = lower_bound(key);
    if(index == 0){
        return value_at(0);
    }
    else if(index == page_hdr->num_key){
        return value_at(page_hdr->num_key-1);
    }
    else{
        return value_at(index);
    }
}

/**
 * @brief 在指定位置插入n个连续的键值对
 * 将key的前n位插入到原来keys中的pos位置；将rid的前n位插入到原来rids中的pos位置
 * 该函数应该是作为更高层函数的基础功能组件
 * @param pos 要插入键值对的位置
 * @param (key, rid) 连续键值对的起始地址，也就是第一个键值对，可以通过(key, rid)来获取n个键值对
 * @param n 键值对数量
 * @note [0,pos)           [pos,num_key)
 *                            key_slot
 *                            /      \
 *                           /        \
 *       [0,pos)     [pos,pos+n)   [pos+n,num_key+n)
 *                      key           key_slot
 */
void IxNodeHandle::insert_pairs(int pos, const char *key, const Rid *rid, int n) {
    // Todo:
    // 1. 判断pos的合法性
    // 2. 通过key获取n个连续键值对的key值，并把n个key值插入到pos位置
    // 3. 通过rid获取n个连续键值对的rid值，并把n个rid值插入到pos位置
    // 4. 更新当前节点的键数量
    if (pos < 0 || pos > page_hdr->num_key) {
        throw IndexEntryNotFoundError();
    }

    // 移动键值对
    auto&& num_key = page_hdr->num_key;
    auto&& col_tot_len = file_hdr->col_tot_len_;
    auto&& rid_begin = get_rid(pos);
    auto&& key_begin = get_key(pos);
    memmove(rid_begin + n, rid_begin, (num_key-pos) * sizeof(Rid));
    memmove(key_begin + n*col_tot_len, key_begin, (num_key-pos) * col_tot_len);

    // 增加插入的键值对
    for(int i = 0; i < n; i ++){
        set_rid(pos + i, *(rid + i));
        set_key(pos + i, key + i * col_tot_len);
    }

    // 更新键值对数目
    page_hdr->num_key += n;
}

/**
 * @brief 用于在结点中插入单个键值对。
 * 函数返回插入后的键值对数量
 *
 * @param (key, value) 要插入的键值对
 * @return int 键值对数量
 */
int IxNodeHandle::insert(const char *key, const Rid &value) {
    // Todo:
    // 1. 查找要插入的键值对应该插入到当前节点的哪个位置
    // 2. 如果key重复则不插入
    // 3. 如果key不重复则插入键值对
    // 4. 返回完成插入操作之后的键值对数量
    const int PAIR_NUM = 1;

    // 查询插入到哪个位置
    int pos = lower_bound(key);

    // 若不重复则插入该键值对
    if(pos >= page_hdr->num_key || memcmp(get_key(pos), key, file_hdr->col_tot_len_) != 0)
        insert_pairs(pos, key, &value, PAIR_NUM);

    // 返回插入操作之后的键值对数量
    return page_hdr->num_key;
}

/**
 * @brief 用于在结点中的指定位置删除单个键值对
 *
 * @param pos 要删除键值对的位置
 */
void IxNodeHandle::erase_pair(int pos) {
    // Todo:
    // 1. 删除该位置的key
    // 2. 删除该位置的rid
    // 3. 更新结点的键值对数量
    if(pos < 0 || pos >= page_hdr->num_key)
        throw IndexEntryNotFoundError();
    auto&& num_key = page_hdr->num_key;
    auto&& col_tot_len = file_hdr->col_tot_len_;
    auto&& rid_begin = get_rid(pos);
    auto&& key_begin = get_key(pos);

    // 直接移动覆盖实现删除
    memmove(rid_begin, rid_begin + 1, (num_key-pos-1) * sizeof(Rid));
    memmove(key_begin, key_begin + col_tot_len, (num_key-pos-1) * col_tot_len);

    // 更新键值对数量
    page_hdr->num_key --;
}

/**
 * @brief 用于在结点中删除指定key的键值对。函数返回删除后的键值对数量
 *
 * @param key 要删除的键值对key值
 * @return 完成删除操作后的键值对数量
 */
int IxNodeHandle::remove(const char *key) {
    // Todo:
    // 1. 查找要删除键值对的位置
    // 2. 如果要删除的键值对存在，删除键值对
    // 3. 返回完成删除操作后的键值对数量

    // 查找哪里需要删除
    int pos = lower_bound(key);

    // 如果存在则删除
    if(pos < page_hdr->num_key && memcmp(get_key(pos), key, file_hdr->col_tot_len_) == 0) {
        erase_pair(pos);
    }
    return page_hdr->num_key;
}

IxIndexHandle::IxIndexHandle(DiskManager *disk_manager, BufferPoolManager *buffer_pool_manager, int fd)
    : disk_manager_(disk_manager), buffer_pool_manager_(buffer_pool_manager), fd_(fd) {
    // init file_hdr_
    disk_manager_->read_page(fd, IX_FILE_HDR_PAGE, (char *)&file_hdr_, sizeof(file_hdr_));
    char* buf = new char[PAGE_SIZE];
    memset(buf, 0, PAGE_SIZE);
    disk_manager_->read_page(fd, IX_FILE_HDR_PAGE, buf, PAGE_SIZE);
    file_hdr_ = new IxFileHdr();
    file_hdr_->deserialize(buf);
    
    // disk_manager管理的fd对应的文件中，设置从file_hdr_->num_pages开始分配page_no
    int now_page_no = disk_manager_->get_fd2pageno(fd);
    disk_manager_->set_fd2pageno(fd, now_page_no + 1);
}

/**
 * @brief 用于查找指定键所在的叶子结点
 * @param key 要查找的目标key值
 * @param operation 查找到目标键值对后要进行的操作类型
 * @param transaction 事务参数，如果不需要则默认传入nullptr
 * @return [leaf node] and [root_is_latched] 返回目标叶子结点以及根结点是否加锁
 * @note need to Unlatch and unpin the leaf node outside!
 * 注意：用了FindLeafPage之后一定要unlatch叶结点，否则下次latch该结点会堵塞！
 */
std::pair<IxNodeHandle *, bool> IxIndexHandle::find_leaf_page(const char *key, Operation operation,
                                                            Transaction *transaction, bool find_first) {
    // Todo:
    // 1. 获取根节点
    // 2. 从根节点开始不断向下查找目标key
    // 3. 找到包含该key值的叶子结点停止查找，并返回叶子节点

    // BUG: 需要添加并发
    root_latch_.lock();
    // 获取根节点
    IxNodeHandle* node = fetch_node(file_hdr_->root_page_);
    bool root_is_latched = false;

    // 向下查找目标key
    while(!node->is_leaf_page()){
        auto&& child_page_no = node->internal_lookup(key);
        IxNodeHandle* child_node = fetch_node(child_page_no);
        buffer_pool_manager_->unpin_page(node->get_page_id(), false);
        node = std::move(child_node);
    }

    // 返回
    return std::make_pair(node, root_is_latched);
}

/**
 * @brief 用于查找指定键在叶子结点中的对应的值result
 *
 * @param key 查找的目标key值
 * @param result 用于存放结果的容器
 * @param transaction 事务指针
 * @return bool 返回目标键值对是否存在
 */
bool IxIndexHandle::get_value(const char *key, std::vector<Rid> *result, Transaction *transaction) {
    // Todo:
    // 1. 获取目标key值所在的叶子结点
    // 2. 在叶子节点中查找目标key值的位置，并读取key对应的rid
    // 3. 把rid存入result参数中
    // 提示：使用完buffer_pool提供的page之后，记得unpin page；记得处理并发的上锁

    // BUG: 需要添加并发

    // 查找叶子节点
    auto &&[leaf_node, root_is_latched] = find_leaf_page(key, Operation::FIND, transaction);

    // 如果找不到叶子节点
    if(leaf_node == nullptr){
        return false;
    }

    Rid* rid = nullptr;
    bool found = leaf_node->leaf_lookup(key, &rid);

    // 如果找到了对应的rid
    if(found && rid != nullptr){
        result->emplace_back(*rid);
    }

    // 释放叶子节点
    buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), false);
    return found;
}

/**
 * @brief  将传入的一个node拆分(Split)成两个结点，在node的右边生成一个新结点new node
 * @param node 需要拆分的结点
 * @return 拆分得到的new_node
 * @note need to unpin the new node outside
 * 注意：本函数执行完毕后，原node和new node都需要在函数外面进行unpin
 */
IxNodeHandle *IxIndexHandle::split(IxNodeHandle *node) {
    // Todo:
    // 1. 将原结点的键值对平均分配，右半部分分裂为新的右兄弟结点
    //    需要初始化新节点的page_hdr内容
    // 2. 如果新的右兄弟结点是叶子结点，更新新旧节点的prev_leaf和next_leaf指针
    //    为新节点分配键值对，更新旧节点的键值对数记录
    // 3. 如果新的右兄弟结点不是叶子结点，更新该结点的所有孩子结点的父节点信息(使用IxIndexHandle::maintain_child())

    // 平均分配并分裂出新节点
    auto new_node = create_node();
    // 初始化新结点页头信息
    new_node->page_hdr->is_leaf = node->page_hdr->is_leaf;
    new_node->page_hdr->parent = node->page_hdr->parent;
    new_node->page_hdr->num_key = 0;
    // 分裂节点
    int mid = node->page_hdr->num_key / 2;
    int num_key_to_move = node->page_hdr->num_key - mid;
    // 后半部分移动到新结点
    auto key_begin = node->get_key(mid);
    auto rid_begin = node->get_rid(mid);
    // 插入到新结点开始位置
    new_node->insert_pairs(0, key_begin, rid_begin, num_key_to_move);
    node->page_hdr->num_key = mid;
    // 如果原节点是叶子节点
    if(node->is_leaf_page()){
        // 更新指针
        new_node->set_prev_leaf(node->get_page_no());
        new_node->set_next_leaf(node->get_page_no());
        node->set_next_leaf(new_node->get_page_no());

        // 指向成环，最后一个节点指向第一个节点，因此必须更新
        auto next_leaf = fetch_node(new_node->get_next_leaf());
        next_leaf->set_prev_leaf(new_node->get_page_no());
        buffer_pool_manager_->unpin_page(next_leaf->get_page_id(), true);
    }
    else{
        // 如果是内部节点，更新所有被移动子节点的父节点信息
        for(int i = 0; i < new_node->page_hdr->num_key; i ++){
            maintain_child(new_node, i);
        }
    }

    return new_node;
}

/**
 * @brief Insert key & value pair into internal page after split
 * 拆分(Split)后，向上找到old_node的父结点
 * 将new_node的第一个key插入到父结点，其位置在 父结点指向old_node的孩子指针 之后
 * 如果插入后>=maxsize，则必须继续拆分父结点，然后在其父结点的父结点再插入，即需要递归
 * 直到找到的old_node为根结点时，结束递归（此时将会新建一个根R，关键字为key，old_node和new_node为其孩子）
 *
 * @param (old_node, new_node) 原结点为old_node，old_node被分裂之后产生了新的右兄弟结点new_node
 * @param key 要插入parent的key
 * @note 一个结点插入了键值对之后需要分裂，分裂后左半部分的键值对保留在原结点，在参数中称为old_node，
 * 右半部分的键值对分裂为新的右兄弟节点，在参数中称为new_node（参考Split函数来理解old_node和new_node）
 * @note 本函数执行完毕后，new node和old node都需要在函数外面进行unpin
 */
void IxIndexHandle::insert_into_parent(IxNodeHandle *old_node, const char *key, IxNodeHandle *new_node,
                                     Transaction *transaction) {
    // Todo:
    // 1. 分裂前的结点（原结点, old_node）是否为根结点，如果为根结点需要分配新的root
    // 2. 获取原结点（old_node）的父亲结点
    // 3. 获取key对应的rid，并将(key, rid)插入到父亲结点
    // 4. 如果父亲结点仍需要继续分裂，则进行递归插入
    // 提示：记得unpin page

    // BUG: 需要添加并发
    if(old_node->is_root_page()){
        // 如果是根节点
        auto new_root = create_node();
        // 初始化新根节点
        new_root->page_hdr->is_leaf = false;
        new_root->page_hdr->parent = IX_NO_PAGE;
        new_root->page_hdr->num_key = 0;
        new_root->page_hdr->prev_leaf = new_root->page_hdr->next_leaf = IX_NO_PAGE;

        // 设置新根节点的第一个key为old_node的第一个key
        new_root->insert_pair(0, old_node->get_key(0), {old_node->get_page_no(), -1});
        new_root->insert_pair(1, key, {new_node->get_page_no(), -1});

        // 维护父子关系
        old_node->set_parent_page_no(new_root->get_page_no());
        new_node->set_parent_page_no(new_root->get_page_no());

        // 更新file_hdr_
        file_hdr_->root_page_ = new_root->get_page_no();

        // 释放节点
        buffer_pool_manager_->unpin_page(new_root->get_page_id(), true);
        return;
    }
    else{
        // 获取父节点
        auto parent = fetch_node(old_node->get_parent_page_no());
        // 获取位置并插入
        auto index = parent->find_child(old_node);
        parent->insert_pair(index+1, key, {new_node->get_page_no(), -1});

        if(parent->get_size() >= parent->get_max_size()){
            // 如果父节点需要分裂
            auto new_parent = split(parent);
            // 获取中间键值作为新的分隔键
            const auto mid = new_parent->get_key(0);
            // 递归插入到父节点
            insert_into_parent(parent, mid, new_parent, transaction);
            // 释放新父节点
            buffer_pool_manager_->unpin_page(new_parent->get_page_id(), true);
        }
    }
}

/**
 * @brief 将指定键值对插入到B+树中
 * @param (key, value) 要插入的键值对
 * @param transaction 事务指针
 * @return page_id_t 插入到的叶结点的page_no
 */
page_id_t IxIndexHandle::insert_entry(const char *key, const Rid &value, Transaction *transaction) {
    // Todo:
    // 1. 查找key值应该插入到哪个叶子节点
    // 2. 在该叶子节点中插入键值对
    // 3. 如果结点已满，分裂结点，并把新结点的相关信息插入父节点
    // 提示：记得unpin page；若当前叶子节点是最右叶子节点，则需要更新file_hdr_.last_leaf；记得处理并发的上锁
//    if(transaction){
//        std::scoped_lock<std::mutex> lock(root_latch_);
//    }
    std::scoped_lock<std::mutex> lock(root_latch_);
    auto&& [leaf_node, root_is_latched] = find_leaf_page(key, Operation::INSERT, transaction);
    if(leaf_node == nullptr){
        return -1;  // 如果没有找到叶子节点，返回-1
    }
    int old_num = leaf_node->get_size();
    int new_num = leaf_node->insert(key, value);
    if(old_num == new_num){
        buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), false);
        return leaf_node->get_page_no();
    }
    if(!memcmp(leaf_node->get_key(0), key, file_hdr_->col_tot_len_))
        maintain_parent(leaf_node);
    // 如果插入后叶子结点的键值对数量超过了最大限制，则需要进行分裂
    if(new_num >= leaf_node->get_max_size()) {
        auto new_node = split(leaf_node);
        insert_into_parent(leaf_node, new_node->get_key(0), new_node, transaction);
        if(file_hdr_->last_leaf_ == leaf_node->get_page_no()){
            file_hdr_->last_leaf_ = new_node->get_page_no();
        }
        buffer_pool_manager_->unpin_page(new_node->get_page_id(), true);
    }
    buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), true);
    return leaf_node->get_page_no();
}

/**
 * @brief 用于删除B+树中含有指定key的键值对
 * @param key 要删除的key值
 * @param transaction 事务指针
 */
bool IxIndexHandle::delete_entry(const char *key, Transaction *transaction) {
    // Todo:
    // 1. 获取该键值对所在的叶子结点
    // 2. 在该叶子结点中删除键值对
    // 3. 如果删除成功需要调用CoalesceOrRedistribute来进行合并或重分配操作，并根据函数返回结果判断是否有结点需要删除
    // 4. 如果需要并发，并且需要删除叶子结点，则需要在事务的delete_page_set中添加删除结点的对应页面；记得处理并发的上锁

    std::scoped_lock<std::mutex> lock(root_latch_);
    auto&& [leaf_node, root_is_latched] = find_leaf_page(key, Operation::DELETE, transaction);
    if(leaf_node == nullptr)
        return false;
    int old_num = leaf_node->get_size();
    int new_num = leaf_node->remove(key);
    if(old_num == new_num){
        buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), false);
        return false;
    }
    if(!memcmp(leaf_node->get_key(0), key, file_hdr_->col_tot_len_))
        maintain_parent(leaf_node);
    bool to_delete = coalesce_or_redistribute(leaf_node, transaction, &root_is_latched);
    if(to_delete){
        if(root_is_latched){
            root_latch_.unlock();
        }
        if(transaction != nullptr){
            transaction->append_index_deleted_page(leaf_node->page);
        }
    }
    buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), true);
    return true;
}

/**
 * @brief 用于处理合并和重分配的逻辑，用于删除键值对后调用
 *
 * @param node 执行完删除操作的结点
 * @param transaction 事务指针
 * @param root_is_latched 传出参数：根节点是否上锁，用于并发操作
 * @return 是否需要删除结点
 * @note User needs to first find the sibling of input page.
 * If sibling's size + input page's size >= 2 * page's minsize, then redistribute.
 * Otherwise, merge(Coalesce).
 */
bool IxIndexHandle::coalesce_or_redistribute(IxNodeHandle *node, Transaction *transaction, bool *root_is_latched) {
    // Todo:
    // 1. 判断node结点是否为根节点
    //    1.1 如果是根节点，需要调用AdjustRoot() 函数来进行处理，返回根节点是否需要被删除
    //    1.2 如果不是根节点，并且不需要执行合并或重分配操作，则直接返回false，否则执行2
    // 2. 获取node结点的父亲结点
    // 3. 寻找node结点的兄弟结点（优先选取前驱结点）
    // 4. 如果node结点和兄弟结点的键值对数量之和，能够支撑两个B+树结点（即node.size+neighbor.size >=
    // NodeMinSize*2)，则只需要重新分配键值对（调用Redistribute函数）
    // 5. 如果不满足上述条件，则需要合并两个结点，将右边的结点合并到左边的结点（调用Coalesce函数）

    // BUG: 需要添加并发
    // 如果是根节点
    if(node->is_root_page()){
        bool ret = adjust_root(node);
        if(*root_is_latched){
            root_latch_.unlock();
            *root_is_latched = false;
        }
        return ret;
    }

    // 大于等于半满则不需要重新分配
    if(node->get_size() >= node->get_min_size()){
        return false;
    }

    auto parent = fetch_node(node->get_parent_page_no());
    auto index = parent->find_child(node);
    IxNodeHandle* neighbor = nullptr;
    if(!index){
        // 如果在第零个位置，则取后继节点
        neighbor = fetch_node(parent->value_at(1));
    }
    else{
        // 否则取前驱节点
        neighbor = fetch_node(parent->value_at(index-1));
    }

    // 如果兄弟结点的大小加上node结点的大小大于等于2*min_size，则进行重分配
    if((node->get_size() + neighbor->get_size()) >= (node->get_min_size() << 1)){
        if(*root_is_latched){
            root_latch_.unlock();
            *root_is_latched = false;
        }
        redistribute(neighbor, node, parent, index);
        buffer_pool_manager_->unpin_page(parent->get_page_id(), true);
        buffer_pool_manager_->unpin_page(node->get_page_id(), true);
        return false; // 不需要删除结点
    }
    else{
        // 否则合并
        bool ret = coalesce(&neighbor, &node, &parent, index, transaction, root_is_latched);
        if(transaction != nullptr)
            transaction->append_index_deleted_page(parent->page);
        buffer_pool_manager_->unpin_page(parent->get_page_id(), true);
        buffer_pool_manager_->unpin_page(node->get_page_id(), true);
        return ret; // 返回是否需要删除结点
    }
}

/**
 * @brief 用于当根结点被删除了一个键值对之后的处理
 * @param old_root_node 原根节点
 * @return bool 根结点是否需要被删除
 * @note size of root page can be less than min size and this method is only called within coalesce_or_redistribute()
 */
bool IxIndexHandle::adjust_root(IxNodeHandle *old_root_node) {
    // Todo:
    // 1. 如果old_root_node是内部结点，并且大小为1，则直接把它的孩子更新成新的根结点
    // 2. 如果old_root_node是叶结点，且大小为0，则直接更新root page
    // 3. 除了上述两种情况，不需要进行操作
    if(old_root_node->is_internal_page() && old_root_node->get_size() == 1){
        // 如果是内部结点且大小为1
        auto child_no = old_root_node->value_at(0);
        auto child = fetch_node(child_no);
        child->set_parent_page_no(IX_NO_PAGE);
        file_hdr_->root_page_ = child->get_page_no();
        buffer_pool_manager_->unpin_page(child->get_page_id(), true);
        release_node_handle(*old_root_node);
        return true;
    }
    else if(old_root_node->is_leaf_page() && old_root_node->get_size() == 0){
        // 如果是叶结点且大小为0
        file_hdr_->root_page_ = IX_INIT_ROOT_PAGE;
        release_node_handle(*old_root_node);
        // 更新最后一个叶子结点
        // BUG: 可能需要删除
        file_hdr_->first_leaf_ = IX_NO_PAGE;
        file_hdr_->last_leaf_ = IX_NO_PAGE;
        return true;
    }
    return false;
}

/**
 * @brief 重新分配node和兄弟结点neighbor_node的键值对
 * Redistribute key & value pairs from one page to its sibling page. If index == 0, move sibling page's first key
 * & value pair into end of input "node", otherwise move sibling page's last key & value pair into head of input "node".
 *
 * @param neighbor_node sibling page of input "node"
 * @param node input from method coalesceOrRedistribute()
 * @param parent the parent of "node" and "neighbor_node"
 * @param index node在parent中的rid_idx
 * @note node是之前刚被删除过一个key的结点
 * index=0，则neighbor是node后继结点，表示：node(left)      neighbor(right)
 * index>0，则neighbor是node前驱结点，表示：neighbor(left)  node(right)
 * 注意更新parent结点的相关kv对
 */
void IxIndexHandle::redistribute(IxNodeHandle *neighbor_node, IxNodeHandle *node, IxNodeHandle *parent, int index) {
    // Todo:
    // 1. 通过index判断neighbor_node是否为node的前驱结点
    // 2. 从neighbor_node中移动一个键值对到node结点中
    // 3. 更新父节点中的相关信息，并且修改移动键值对对应孩字结点的父结点信息（maintain_child函数）
    // 注意：neighbor_node的位置不同，需要移动的键值对不同，需要分类讨论
    if(index == 0){
        // 如果是后继节点
        const char* neighbor_key = neighbor_node->get_key(0);
        Rid* neighbor_rid = neighbor_node->get_rid(0);
        // 将neighbor_node的第一个键值对移动到node结点的末尾
        node->insert_pair(node->get_size(), neighbor_key, *neighbor_rid);
        neighbor_node->erase_pair(0);
        parent->set_key(index+1, neighbor_node->get_key(0));
        maintain_child(node, node->get_size()-1);
    }
    else{
        const char* neighbor_key = neighbor_node->get_key(neighbor_node->get_size()-1);
        Rid* neighbor_rid = neighbor_node->get_rid(neighbor_node->get_size()-1);
        node->insert_pair(0, neighbor_key, *neighbor_rid);
        neighbor_node->erase_pair(neighbor_node->get_size()-1);
        parent->set_key(index, neighbor_node->get_key(0));
        maintain_child(node, 0);
    }
}

/**
 * @brief 合并(Coalesce)函数是将node和其直接前驱进行合并，也就是和它左边的neighbor_node进行合并；
 * 假设node一定在右边。如果上层传入的index=0，说明node在左边，那么交换node和neighbor_node，保证node在右边；合并到左结点，实际上就是删除了右结点；
 * Move all the key & value pairs from one page to its sibling page, and notify buffer pool manager to delete this page.
 * Parent page must be adjusted to take info of deletion into account. Remember to deal with coalesce or redistribute
 * recursively if necessary.
 *
 * @param neighbor_node sibling page of input "node" (neighbor_node是node的前结点)
 * @param node input from method coalesceOrRedistribute() (node结点是需要被删除的)
 * @param parent parent page of input "node"
 * @param index node在parent中的rid_idx
 * @return true means parent node should be deleted, false means no deletion happend
 * @note Assume that *neighbor_node is the left sibling of *node (neighbor -> node)
 */
bool IxIndexHandle::coalesce(IxNodeHandle **neighbor_node, IxNodeHandle **node, IxNodeHandle **parent, int index,
                             Transaction *transaction, bool *root_is_latched) {
    // Todo:
    // 1. 用index判断neighbor_node是否为node的前驱结点，若不是则交换两个结点，让neighbor_node作为左结点，node作为右结点
    // 2. 把node结点的键值对移动到neighbor_node中，并更新node结点孩子结点的父节点信息（调用maintain_child函数）
    // 3. 释放和删除node结点，并删除parent中node结点的信息，返回parent是否需要被删除
    // 提示：如果是叶子结点且为最右叶子结点，需要更新file_hdr_.last_leaf

    if(index == 0){
        // 如果是后继节点，则交换
        std::swap(*neighbor_node, *node);
        index = 1;
    }
    IxNodeHandle* _neighbor = *neighbor_node;
    IxNodeHandle* _node = *node;
    IxNodeHandle* _parent = *parent;
    // 将node结点的所有键值对移动到neighbor_node中
    auto neighbor_size = _neighbor->get_size();
    _neighbor->insert_pairs(neighbor_size, _node->keys, _node->rids, _node->get_size());
    // 内部节点更新所有孩子结点的父节点信息
    if(_node->is_internal_page()){
        for(auto i = neighbor_size; i < _neighbor->get_size(); i ++){
            maintain_child(_neighbor, i);
        }
    }
    else{
        if(_node->get_page_no() == file_hdr_->last_leaf_){
            // 如果是最右叶子结点，更新file_hdr_.last_leaf
            file_hdr_->last_leaf_ = _neighbor->get_page_no();
        }
        erase_leaf(_node);
    }

    // 释放node结点
    transaction->append_index_deleted_page(_node->page);
    release_node_handle(*_node);
    // 更新parent结点
    _parent->erase_pair(index);
    return coalesce_or_redistribute(_parent, transaction, root_is_latched);
}


// 自定义函数，判断有无索引为key
bool IxIndexHandle::has_key(const char *key, Transaction *transaction) {
    // TODO: 
}

/**
 * @brief 这里把iid转换成了rid，即iid的slot_no作为node的rid_idx(key_idx)
 * node其实就是把slot_no作为键值对数组的下标
 * 换而言之，每个iid对应的索引槽存了一对(key,rid)，指向了(要建立索引的属性首地址,插入/删除记录的位置)
 *
 * @param iid
 * @return Rid
 * @note iid和rid存的不是一个东西，rid是上层传过来的记录位置，iid是索引内部生成的索引槽位置
 */
Rid IxIndexHandle::get_rid(const Iid &iid) const {
    IxNodeHandle *node = fetch_node(iid.page_no);
    if (iid.slot_no >= node->get_size()) {
        throw IndexEntryNotFoundError();
    }
    buffer_pool_manager_->unpin_page(node->get_page_id(), false);  // unpin it!
    return *node->get_rid(iid.slot_no);
}

/**
 * @brief FindLeafPage + lower_bound
 *
 * @param key
 * @return Iid
 * @note 上层传入的key本来是int类型，通过(const char *)&key进行了转换
 * 可用*(int *)key转换回去
 */
Iid IxIndexHandle::lower_bound(const char *key) {
    auto&& [leaf_node, root_is_latched] = find_leaf_page(key, Operation::FIND, nullptr, false);
    auto pos = leaf_node->lower_bound(key);
    if(pos == leaf_node->get_size() && leaf_node->get_page_no() != file_hdr_->last_leaf_) {
        Iid ret{leaf_node->get_next_leaf(), 0};
        buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), false);
        return ret;
    }
    else if(pos == leaf_node->get_size() && leaf_node->get_page_no() == file_hdr_->last_leaf_){
        Iid ret{leaf_node->get_page_no(), pos};
        buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), false);
        return ret;
    }
    else{
        Iid ret{leaf_node->get_page_no(), pos};
        buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), false);
        return ret;
    }
}

/**
 * @brief FindLeafPage + upper_bound
 *
 * @param key
 * @return Iid
 */
Iid IxIndexHandle::upper_bound(const char *key) {
    auto&& [leaf_node, root_is_latched] = find_leaf_page(key, Operation::FIND, nullptr, false);
    auto pos = leaf_node->upper_bound(key);
    // 如果第0个key也大于key
    if(pos == 1 && ix_compare(key, leaf_node->get_key(0), file_hdr_->col_types_, file_hdr_->col_lens_) < 0){
        pos = 0;
    }
    if(pos == leaf_node->get_size() && leaf_node->get_page_no() != file_hdr_->last_leaf_){
        Iid ret{leaf_node->get_next_leaf(), 0};
        buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), false);
        return ret;
    }
    else if(pos == leaf_node->get_size() && leaf_node->get_page_no() == file_hdr_->last_leaf_){
        Iid ret{leaf_node->get_page_no(), pos};
        buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), false);
        return ret;
    }
    else{
        Iid ret{leaf_node->get_page_no(), pos};
        buffer_pool_manager_->unpin_page(leaf_node->get_page_id(), false);
        return ret;
    }
}

/**
 * @brief 指向最后一个叶子的最后一个结点的后一个
 * 用处在于可以作为IxScan的最后一个
 *
 * @return Iid
 */
Iid IxIndexHandle::leaf_end() const {
    IxNodeHandle *node = fetch_node(file_hdr_->last_leaf_);
    Iid iid = {.page_no = file_hdr_->last_leaf_, .slot_no = node->get_size()};
    buffer_pool_manager_->unpin_page(node->get_page_id(), false);  // unpin it!
    return iid;
}

/**
 * @brief 指向第一个叶子的第一个结点
 * 用处在于可以作为IxScan的第一个
 *
 * @return Iid
 */
Iid IxIndexHandle::leaf_begin() const {
    Iid iid = {.page_no = file_hdr_->first_leaf_, .slot_no = 0};
    return iid;
}

/**
 * @brief 获取一个指定结点
 *
 * @param page_no
 * @return IxNodeHandle*
 * @note pin the page, remember to unpin it outside!
 */
IxNodeHandle *IxIndexHandle::fetch_node(int page_no) const {
    Page *page = buffer_pool_manager_->fetch_page(PageId{fd_, page_no});
    IxNodeHandle *node = new IxNodeHandle(file_hdr_, page);
    
    return node;
}

/**
 * @brief 创建一个新结点
 *
 * @return IxNodeHandle*
 * @note pin the page, remember to unpin it outside!
 * 注意：对于Index的处理是，删除某个页面后，认为该被删除的页面是free_page
 * 而first_free_page实际上就是最新被删除的页面，初始为IX_NO_PAGE
 * 在最开始插入时，一直是create node，那么first_page_no一直没变，一直是IX_NO_PAGE
 * 与Record的处理不同，Record将未插入满的记录页认为是free_page
 */
IxNodeHandle *IxIndexHandle::create_node() {
    IxNodeHandle *node;
    file_hdr_->num_pages_++;

    PageId new_page_id = {.fd = fd_, .page_no = INVALID_PAGE_ID};
    // 从3开始分配page_no，第一次分配之后，new_page_id.page_no=3，file_hdr_.num_pages=4
    Page *page = buffer_pool_manager_->new_page(&new_page_id);
    node = new IxNodeHandle(file_hdr_, page);
    return node;
}

/**
 * @brief 从node开始更新其父节点的第一个key，一直向上更新直到根节点
 *
 * @param node
 */
void IxIndexHandle::maintain_parent(IxNodeHandle *node) {
    IxNodeHandle *curr = node;
    while (curr->get_parent_page_no() != IX_NO_PAGE) {
        // Load its parent
        IxNodeHandle *parent = fetch_node(curr->get_parent_page_no());
        int rank = parent->find_child(curr);
        char *parent_key = parent->get_key(rank);
        char *child_first_key = curr->get_key(0);
        if (memcmp(parent_key, child_first_key, file_hdr_->col_tot_len_) == 0) {
            assert(buffer_pool_manager_->unpin_page(parent->get_page_id(), true));
            break;
        }
        memcpy(parent_key, child_first_key, file_hdr_->col_tot_len_);  // 修改了parent node
        curr = parent;

        assert(buffer_pool_manager_->unpin_page(parent->get_page_id(), true));
    }
}

/**
 * @brief 要删除leaf之前调用此函数，更新leaf前驱结点的next指针和后继结点的prev指针
 *
 * @param leaf 要删除的leaf
 */
void IxIndexHandle::erase_leaf(IxNodeHandle *leaf) {
    assert(leaf->is_leaf_page());

    IxNodeHandle *prev = fetch_node(leaf->get_prev_leaf());
    prev->set_next_leaf(leaf->get_next_leaf());
    buffer_pool_manager_->unpin_page(prev->get_page_id(), true);

    IxNodeHandle *next = fetch_node(leaf->get_next_leaf());
    next->set_prev_leaf(leaf->get_prev_leaf());  // 注意此处是SetPrevLeaf()
    buffer_pool_manager_->unpin_page(next->get_page_id(), true);
}

/**
 * @brief 删除node时，更新file_hdr_.num_pages
 *
 * @param node
 */
void IxIndexHandle::release_node_handle(IxNodeHandle &node) {
    file_hdr_->num_pages_--;
}

/**
 * @brief 将node的第child_idx个孩子结点的父节点置为node
 */
void IxIndexHandle::maintain_child(IxNodeHandle *node, int child_idx) {
    if (!node->is_leaf_page()) {
        //  Current node is inner node, load its child and set its parent to current node
        int child_page_no = node->value_at(child_idx);
        IxNodeHandle *child = fetch_node(child_page_no);
        child->set_parent_page_no(node->get_page_no());
        buffer_pool_manager_->unpin_page(child->get_page_id(), true);
    }
}

