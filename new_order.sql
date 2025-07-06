-- 创建TPC-C简化模式
-- 所有表都不包含主键约束，只使用基本数据类型

-- 创建仓库表
CREATE TABLE warehouse (
    w_id INT,           -- 仓库ID
    w_name CHAR(10),    -- 仓库名称
    w_ytd FLOAT         -- 年至今销售额
);

-- 创建区域表
CREATE TABLE district (
    d_id INT,           -- 区域ID
    d_w_id INT,         -- 所属仓库ID
    d_name CHAR(10),    -- 区域名称
    d_ytd FLOAT,        -- 年至今销售额
    d_next_o_id INT     -- 下一个订单ID
);

-- 创建客户表
CREATE TABLE customer (
    c_id INT,           -- 客户ID
    c_d_id INT,         -- 所属区域ID
    c_w_id INT,         -- 所属仓库ID
    c_first CHAR(16),   -- 名
    c_last CHAR(16),    -- 姓
    c_balance FLOAT     -- 余额
);

-- 创建订单表
CREATE TABLE orders (
    o_id INT,           -- 订单ID
    o_d_id INT,         -- 区域ID
    o_w_id INT,         -- 仓库ID
    o_c_id INT,         -- 客户ID
    o_entry_d CHAR(30), -- 创建日期(简化为CHAR)
    o_ol_cnt INT        -- 订单行数
);

-- 创建新订单表
CREATE TABLE new_order (
    no_o_id INT,        -- 订单ID
    no_d_id INT,        -- 区域ID
    no_w_id INT         -- 仓库ID
);

-- 创建订单行表
CREATE TABLE order_line (
    ol_o_id INT,        -- 订单ID
    ol_d_id INT,        -- 区域ID
    ol_w_id INT,        -- 仓库ID
    ol_number INT,      -- 行号
    ol_i_id INT,        -- 商品ID
    ol_quantity INT,    -- 数量
    ol_amount FLOAT,    -- 金额
    ol_dist_info CHAR(24) -- 分销信息
);

-- 创建商品表
CREATE TABLE item (
    i_id INT,           -- 商品ID
    i_name CHAR(24),    -- 商品名称
    i_price FLOAT       -- 价格
);

-- 创建库存表
CREATE TABLE stock (
    s_i_id INT,         -- 商品ID
    s_w_id INT,         -- 仓库ID
    s_quantity INT,     -- 数量
    s_ytd INT,          -- 年至今销售量
    s_data CHAR(50)     -- 库存数据
);

-- 创建索引以加速查询
CREATE INDEX warehouse (w_id);
CREATE INDEX district (d_id, d_w_id);
CREATE INDEX customer (c_id, c_d_id, c_w_id);
CREATE INDEX orders (o_id, o_d_id, o_w_id);
CREATE INDEX new_order (no_o_id, no_d_id, no_w_id);
CREATE INDEX order_line (ol_o_id, ol_d_id, ol_w_id,ol_number);
CREATE INDEX item (i_id);
CREATE INDEX stock (s_i_id, s_w_id);

-- 初始化测试数据
-- 插入仓库数据
BEGIN;
INSERT INTO warehouse VALUES (1, 'Warehouse1', 300000.00);
INSERT INTO warehouse VALUES (2, 'Warehouse2', 250000.00);
COMMIT;

-- 插入区域数据
BEGIN;
INSERT INTO district VALUES (1, 1, 'District1', 150000.00, 3001);
INSERT INTO district VALUES (2, 1, 'District2', 130000.00, 2001);
INSERT INTO district VALUES (1, 2, 'District3', 170000.00, 4001);
INSERT INTO district VALUES (2, 2, 'District4', 140000.00, 3501);
COMMIT;

-- 插入客户数据
BEGIN;
INSERT INTO customer VALUES (101, 1, 1, 'John', 'Smith', 1000.00);
INSERT INTO customer VALUES (102, 1, 1, 'Mary', 'Jones', 2500.00);
INSERT INTO customer VALUES (103, 2, 1, 'Robert', 'Brown', 750.00);
INSERT INTO customer VALUES (104, 1, 2, 'Linda', 'Wilson', 3200.00);
INSERT INTO customer VALUES (105, 2, 2, 'Michael', 'Taylor', 1800.00);
COMMIT;

-- 插入商品数据
BEGIN;
INSERT INTO item VALUES (1001, 'Computer', 999.99);
INSERT INTO item VALUES (1002, 'Printer', 299.99);
INSERT INTO item VALUES (1003, 'Monitor', 199.99);
INSERT INTO item VALUES (1004, 'Keyboard', 49.99);
INSERT INTO item VALUES (1005, 'Mouse', 29.99);
INSERT INTO item VALUES (1006, 'Headphones', 79.99);
COMMIT;

-- 插入库存数据
BEGIN;
INSERT INTO stock VALUES (1001, 1, 100, 50, 'Stock data for Computer in W1');
INSERT INTO stock VALUES (1002, 1, 80, 30, 'Stock data for Printer in W1');
INSERT INTO stock VALUES (1003, 1, 120, 40, 'Stock data for Monitor in W1');
INSERT INTO stock VALUES (1004, 1, 200, 100, 'Stock data for Keyboard in W1');
INSERT INTO stock VALUES (1005, 1, 150, 70, 'Stock data for Mouse in W1');
INSERT INTO stock VALUES (1006, 1, 90, 45, 'Stock data for Headphones in W1');

INSERT INTO stock VALUES (1001, 2, 90, 45, 'Stock data for Computer in W2');
INSERT INTO stock VALUES (1002, 2, 70, 25, 'Stock data for Printer in W2');
INSERT INTO stock VALUES (1003, 2, 110, 35, 'Stock data for Monitor in W2');
INSERT INTO stock VALUES (1004, 2, 180, 90, 'Stock data for Keyboard in W2');
INSERT INTO stock VALUES (1005, 2, 140, 65, 'Stock data for Mouse in W2');
INSERT INTO stock VALUES (1006, 2, 80, 40, 'Stock data for Headphones in W2');
COMMIT;

-- TEST 1: 基本NewOrder提交事务
-- 客户101在区域1，仓库1下单，购买3个商品
BEGIN;
-- 1. 获取客户信息，确认客户身份
SELECT c_balance FROM customer WHERE c_id = 101 AND c_d_id = 1 AND c_w_id = 1;

-- 2. 获取区域信息和下一个订单号
SELECT d_next_o_id, d_ytd FROM district WHERE d_id = 1 AND d_w_id = 1;

-- 3. 更新区域表的下一个订单ID
UPDATE district SET d_next_o_id = d_next_o_id + 1 WHERE d_id = 1 AND d_w_id = 1;

-- 4. 创建订单记录
INSERT INTO orders VALUES (3001, 1, 1, 101, '2025-07-06 10:15:00', 3);

-- 5. 创建新订单记录
INSERT INTO new_order VALUES (3001, 1, 1);

-- 6. 为每个商品创建订单行
-- 订单行1: 购买2台电脑
INSERT INTO order_line VALUES (3001, 1, 1, 1, 1001, 2, 1999.98, 'Warehouse 1, District 1');
UPDATE stock SET s_quantity = s_quantity - 2, s_ytd = s_ytd + 2 WHERE s_i_id = 1001 AND s_w_id = 1;

-- 订单行2: 购买1台打印机
INSERT INTO order_line VALUES (3001, 1, 1, 2, 1002, 1, 299.99, 'Warehouse 1, District 1');
UPDATE stock SET s_quantity = s_quantity - 1, s_ytd = s_ytd + 1 WHERE s_i_id = 1002 AND s_w_id = 1;

-- 订单行3: 购买3个鼠标
INSERT INTO order_line VALUES (3001, 1, 1, 3, 1005, 3, 89.97, 'Warehouse 1, District 1');
UPDATE stock SET s_quantity = s_quantity - 3, s_ytd = s_ytd + 3 WHERE s_i_id = 1005 AND s_w_id = 1;

-- 7. 更新仓库年至今销售额
UPDATE warehouse SET w_ytd = w_ytd + 2389.94 WHERE w_id = 1;

-- 8. 更新客户余额
UPDATE customer SET c_balance = c_balance - 2389.94 WHERE c_id = 101 AND c_d_id = 1 AND c_w_id = 1;

COMMIT;

-- 验证提交后的状态
SELECT * FROM orders WHERE o_id = 3001;
SELECT * FROM new_order WHERE no_o_id = 3001;
SELECT * FROM order_line WHERE ol_o_id = 3001;
SELECT s_quantity FROM stock WHERE s_i_id = 1001 AND s_w_id = 1; -- 应该是98
SELECT s_quantity FROM stock WHERE s_i_id = 1002 AND s_w_id = 1; -- 应该是79
SELECT s_quantity FROM stock WHERE s_i_id = 1005 AND s_w_id = 1; -- 应该是147
SELECT w_ytd FROM warehouse WHERE w_id = 1; -- 应该增加了2389.94
SELECT c_balance FROM customer WHERE c_id = 101; -- 应该减少了2389.94

-- TEST 2: NewOrder回滚事务（库存不足）
BEGIN;
-- 1. 获取客户信息
SELECT c_balance FROM customer WHERE c_id = 102 AND c_d_id = 1 AND c_w_id = 1;

-- 2. 获取区域信息
SELECT d_next_o_id, d_ytd FROM district WHERE d_id = 1 AND d_w_id = 1;

-- 3. 更新区域表
UPDATE district SET d_next_o_id = d_next_o_id + 1 WHERE d_id = 1 AND d_w_id = 1;

-- 4. 创建订单
INSERT INTO orders VALUES (3002, 1, 1, 102, '2025-07-06 10:30:00', 2);

-- 5. 创建新订单
INSERT INTO new_order VALUES (3002, 1, 1);

-- 6. 尝试订购过量商品（假设发现库存不足）
INSERT INTO order_line VALUES (3002, 1, 1, 1, 1003, 200, 39998.00, 'Warehouse 1, District 1');
-- 这里检查库存发现不足，需要回滚
SELECT s_quantity FROM stock WHERE s_i_id = 1003 AND s_w_id = 1; -- 只有120个

-- 发现库存不足，回滚整个事务
ABORT;

-- 验证回滚后的状态
SELECT * FROM orders WHERE o_id = 3002; -- 不应该有记录
SELECT * FROM new_order WHERE no_o_id = 3002; -- 不应该有记录
SELECT * FROM order_line WHERE ol_o_id = 3002; -- 不应该有记录
SELECT d_next_o_id FROM district WHERE d_id = 1 AND d_w_id = 1; -- 应该是3002（回滚前的值）

-- TEST 3: 复杂NewOrder提交事务（跨仓库）
BEGIN;
-- 客户104在区域1，仓库2下单，购买多个商品（有些来自仓库1）

-- 1. 获取客户信息
SELECT c_balance FROM customer WHERE c_id = 104 AND c_d_id = 1 AND c_w_id = 2;

-- 2. 获取区域信息和下一个订单号
SELECT d_next_o_id, d_ytd FROM district WHERE d_id = 1 AND d_w_id = 2;

-- 3. 更新区域表
UPDATE district SET d_next_o_id = d_next_o_id + 1 WHERE d_id = 1 AND d_w_id = 2;

-- 4. 创建订单
INSERT INTO orders VALUES (4001, 1, 2, 104, '2025-07-06 11:00:00', 4);

-- 5. 创建新订单
INSERT INTO new_order VALUES (4001, 1, 2);

-- 6. 创建订单行（跨仓库）
-- 本地仓库商品
INSERT INTO order_line VALUES (4001, 1, 2, 1, 1001, 1, 999.99, 'Warehouse 2, District 1');
UPDATE stock SET s_quantity = s_quantity - 1, s_ytd = s_ytd + 1 WHERE s_i_id = 1001 AND s_w_id = 2;

INSERT INTO order_line VALUES (4001, 1, 2, 2, 1003, 2, 399.98, 'Warehouse 2, District 1');
UPDATE stock SET s_quantity = s_quantity - 2, s_ytd = s_ytd + 2 WHERE s_i_id = 1003 AND s_w_id = 2;

-- 远程仓库商品
INSERT INTO order_line VALUES (4001, 1, 2, 3, 1004, 5, 249.95, 'Warehouse 1, District 1');
UPDATE stock SET s_quantity = s_quantity - 5, s_ytd = s_ytd + 5 WHERE s_i_id = 1004 AND s_w_id = 1;

INSERT INTO order_line VALUES (4001, 1, 2, 4, 1006, 2, 159.98, 'Warehouse 1, District 1');
UPDATE stock SET s_quantity = s_quantity - 2, s_ytd = s_ytd + 2 WHERE s_i_id = 1006 AND s_w_id = 1;

-- 7. 更新仓库年至今销售额
UPDATE warehouse SET w_ytd = w_ytd + 1399.97 WHERE w_id = 2;
UPDATE warehouse SET w_ytd = w_ytd + 409.93 WHERE w_id = 1;

-- 8. 更新客户余额
UPDATE customer SET c_balance = c_balance - 1809.90 WHERE c_id = 104 AND c_d_id = 1 AND c_w_id = 2;

COMMIT;

-- 验证提交后的状态
SELECT * FROM orders WHERE o_id = 4001;
SELECT * FROM order_line WHERE ol_o_id = 4001;
SELECT s_quantity FROM stock WHERE s_i_id = 1001 AND s_w_id = 2; -- 应该是89
SELECT s_quantity FROM stock WHERE s_i_id = 1003 AND s_w_id = 2; -- 应该是108
SELECT s_quantity FROM stock WHERE s_i_id = 1004 AND s_w_id = 1; -- 应该是195
SELECT s_quantity FROM stock WHERE s_i_id = 1006 AND s_w_id = 1; -- 应该是88
SELECT c_balance FROM customer WHERE c_id = 104; -- 应该减少了1809.90

-- TEST 4: 高负载回滚测试（多表多行更新）
BEGIN;
-- 尝试创建一个大型订单，然后回滚
-- 1. 更新区域信息
UPDATE district SET d_next_o_id = d_next_o_id + 1 WHERE d_id = 2 AND d_w_id = 2;

-- 2. 创建订单
INSERT INTO orders VALUES (3501, 2, 2, 105, '2025-07-06 12:00:00', 6);
INSERT INTO new_order VALUES (3501, 2, 2);

-- 3. 创建大量订单行并更新库存
INSERT INTO order_line VALUES (3501, 2, 2, 1, 1001, 10, 9999.90, 'Warehouse 2, District 2');
UPDATE stock SET s_quantity = s_quantity - 10, s_ytd = s_ytd + 10 WHERE s_i_id = 1001 AND s_w_id = 2;

INSERT INTO order_line VALUES (3501, 2, 2, 2, 1002, 8, 2399.92, 'Warehouse 2, District 2');
UPDATE stock SET s_quantity = s_quantity - 8, s_ytd = s_ytd + 8 WHERE s_i_id = 1002 AND s_w_id = 2;

INSERT INTO order_line VALUES (3501, 2, 2, 3, 1003, 15, 2999.85, 'Warehouse 2, District 2');
UPDATE stock SET s_quantity = s_quantity - 15, s_ytd = s_ytd + 15 WHERE s_i_id = 1003 AND s_w_id = 2;

INSERT INTO order_line VALUES (3501, 2, 2, 4, 1004, 25, 1249.75, 'Warehouse 2, District 2');
UPDATE stock SET s_quantity = s_quantity - 25, s_ytd = s_ytd + 25 WHERE s_i_id = 1004 AND s_w_id = 2;

INSERT INTO order_line VALUES (3501, 2, 2, 5, 1005, 30, 899.70, 'Warehouse 2, District 2');
UPDATE stock SET s_quantity = s_quantity - 30, s_ytd = s_ytd + 30 WHERE s_i_id = 1005 AND s_w_id = 2;

INSERT INTO order_line VALUES (3501, 2, 2, 6, 1006, 12, 959.88, 'Warehouse 2, District 2');
UPDATE stock SET s_quantity = s_quantity - 12, s_ytd = s_ytd + 12 WHERE s_i_id = 1006 AND s_w_id = 2;

-- 4. 更新其他表
UPDATE warehouse SET w_ytd = w_ytd + 18509.00 WHERE w_id = 2;
UPDATE customer SET c_balance = c_balance - 18509.00 WHERE c_id = 105 AND c_d_id = 2 AND c_w_id = 2;

-- 假设订单出现问题，需要回滚
ABORT;

-- 验证回滚后的状态
SELECT * FROM orders WHERE o_id = 3501; -- 不应该有记录
SELECT * FROM order_line WHERE ol_o_id = 3501; -- 不应该有记录
SELECT s_quantity FROM stock WHERE s_i_id = 1001 AND s_w_id = 2; -- 应该保持不变
SELECT c_balance FROM customer WHERE c_id = 105; -- 应该保持不变
SELECT d_next_o_id FROM district WHERE d_id = 2 AND d_w_id = 2; -- 应该保持不变

-- TEST 5: 极端情况测试（所有表的多次修改和回滚）
BEGIN;
-- 1. 在多个区域和仓库创建订单
-- 区域1，仓库1
UPDATE district SET d_next_o_id = d_next_o_id + 1 WHERE d_id = 1 AND d_w_id = 1;
INSERT INTO orders VALUES (3002, 1, 1, 102, '2025-07-06 14:00:00', 2);
INSERT INTO new_order VALUES (3002, 1, 1);
INSERT INTO order_line VALUES (3002, 1, 1, 1, 1001, 5, 4999.95, 'WH1, District 1');
INSERT INTO order_line VALUES (3002, 1, 1, 2, 1002, 5, 1499.95, 'WH1, District 1');
UPDATE stock SET s_quantity = s_quantity - 5, s_ytd = s_ytd + 5 WHERE s_i_id = 1001 AND s_w_id = 1;
UPDATE stock SET s_quantity = s_quantity - 5, s_ytd = s_ytd + 5 WHERE s_i_id = 1002 AND s_w_id = 1;

-- 区域2，仓库1
UPDATE district SET d_next_o_id = d_next_o_id + 1 WHERE d_id = 2 AND d_w_id = 1;
INSERT INTO orders VALUES (2001, 2, 1, 103, '2025-07-06 14:05:00', 1);
INSERT INTO new_order VALUES (2001, 2, 1);
INSERT INTO order_line VALUES (2001, 2, 1, 1, 1003, 10, 1999.90, 'WH1, District 2');
UPDATE stock SET s_quantity = s_quantity - 10, s_ytd = s_ytd + 10 WHERE s_i_id = 1003 AND s_w_id = 1;

-- 区域1，仓库2
UPDATE district SET d_next_o_id = d_next_o_id + 1 WHERE d_id = 1 AND d_w_id = 2;
INSERT INTO orders VALUES (4002, 1, 2, 104, '2025-07-06 14:10:00', 1);
INSERT INTO new_order VALUES (4002, 1, 2);
INSERT INTO order_line VALUES (4002, 1, 2, 1, 1004, 15, 749.85, 'WH2, District 1');
UPDATE stock SET s_quantity = s_quantity - 15, s_ytd = s_ytd + 15 WHERE s_i_id = 1004 AND s_w_id = 2;

-- 更新所有客户余额和仓库销售额
UPDATE warehouse SET w_ytd = w_ytd + 6499.90 WHERE w_id = 1;
UPDATE warehouse SET w_ytd = w_ytd + 749.85 WHERE w_id = 2;
UPDATE customer SET c_balance = c_balance - 6499.90 WHERE c_id = 102 AND c_d_id = 1 AND c_w_id = 1;
UPDATE customer SET c_balance = c_balance - 1999.90 WHERE c_id = 103 AND c_d_id = 2 AND c_w_id = 1;
UPDATE customer SET c_balance = c_balance - 749.85 WHERE c_id = 104 AND c_d_id = 1 AND c_w_id = 2;

-- 假设系统崩溃，事务需要回滚
ABORT;

-- 验证回滚效果
SELECT * FROM orders WHERE o_id = 3002; -- 不应该有记录
SELECT * FROM order_line WHERE ol_o_id = 3002; -- 不应该有记录
SELECT d_next_o_id FROM district WHERE d_id = 1 AND d_w_id = 1; -- 应保持原值
SELECT d_next_o_id FROM district WHERE d_id = 2 AND d_w_id = 1; -- 应保持原值
SELECT d_next_o_id FROM district WHERE d_id = 1 AND d_w_id = 2; -- 应保持原值
-- 修复：分别查询每个客户的余额，而不是使用IN操作符
SELECT c_balance FROM customer WHERE c_id = 102;
SELECT c_balance FROM customer WHERE c_id = 103;
SELECT c_balance FROM customer WHERE c_id = 104;