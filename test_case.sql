-- -- -- 清理旧表
-- -- DROP TABLE warehouse;
-- -- DROP TABLE district;
-- -- DROP TABLE customer;
-- -- DROP TABLE orders;
-- -- DROP TABLE order_line;
-- -- DROP TABLE item;
-- -- DROP TABLE stock;

-- -- 创建表结构（包含int、float、string、date类型）
-- CREATE TABLE warehouse (w_id INT,w_name CHAR(16),w_street CHAR(32),w_city CHAR(16),w_tax FLOAT,w_ytd FLOAT,w_created DATE);
-- CREATE INDEX warehouse(w_id);

-- CREATE TABLE district (d_id INT,d_w_id INT,d_name CHAR(16),d_tax FLOAT,d_next_o_id INT,d_created DATE);
-- CREATE INDEX district(d_w_id, d_id);

-- CREATE TABLE customer (c_id INT,c_d_id INT,c_w_id INT,c_first CHAR(16),c_last CHAR(16),c_credit CHAR(4),c_balance FLOAT,c_ytd_payment FLOAT,c_since DATE);
-- CREATE INDEX customer(c_d_id, c_id);
-- CREATE INDEX customer(c_last);

-- CREATE TABLE orders (o_id INT,o_d_id INT,o_w_id INT,o_c_id INT,o_entry_d DATE,o_carrier_id INT,o_ol_cnt INT,o_all_local INT);
-- CREATE INDEX orders(o_w_id, o_d_id, o_id);
-- CREATE INDEX orders(o_w_id, o_d_id, o_c_id);

-- CREATE TABLE order_line (ol_o_id INT,ol_d_id INT,ol_w_id INT,ol_number INT,ol_i_id INT,ol_supply_w_id INT,ol_delivery_d DATE,ol_quantity INT,ol_amount FLOAT);
-- CREATE INDEX order_line(ol_w_id, ol_d_id, ol_o_id);

-- CREATE TABLE item (i_id INT,i_name CHAR(32),i_price FLOAT,i_data CHAR(64));
-- CREATE INDEX item(i_id);

-- CREATE TABLE stock (s_i_id INT,s_w_id INT,s_quantity INT,s_ytd INT,s_data CHAR(64),s_dist_01 CHAR(24),s_dist_02 CHAR(24));
-- CREATE INDEX stock(s_w_id, s_i_id);

-- -- 插入初始数据
-- INSERT INTO warehouse VALUES (1, 'Warehouse-001', '123 Main St', 'Seattle', 0.05, 100000.00, '2023-01-01 08:00:00');
-- INSERT INTO warehouse VALUES (2, 'Warehouse-002', '456 Elm St', 'Portland', 0.06, 200000.00, '2023-01-02 09:30:00');

-- INSERT INTO district VALUES (1, 1, 'North', 0.05, 3001, '2023-01-03 10:15:00');
-- INSERT INTO district VALUES (2, 1, 'South', 0.05, 3001, '2023-01-03 10:20:00');
-- INSERT INTO district VALUES (1, 2, 'East', 0.06, 3001, '2023-01-04 11:00:00');

-- INSERT INTO customer VALUES (1, 1, 1, 'Alice', 'Johnson', 'GC', 1000.00, 0.00, '2023-01-05 14:30:00');
-- INSERT INTO customer VALUES (2, 1, 1, 'Bob', 'Smith', 'BC', 500.00, 0.00, '2023-01-06 09:15:00');
-- INSERT INTO customer VALUES (3, 2, 1, 'Carol', 'Davis', 'GC', 1500.00, 0.00, '2023-01-07 16:45:00');
-- INSERT INTO customer VALUES (1, 3, 2, 'David', 'Miller', 'GC', 2000.00, 0.00, '2023-01-08 11:20:00');

-- -- 测试项目1：基本事务提交 - 新订单创建
-- BEGIN;
-- -- 创建新订单
-- INSERT INTO orders VALUES (3001, 1, 1, 1, '2025-06-20 09:30:00', 0, 2, 1);
-- -- 创建订单明细
-- INSERT INTO order_line VALUES (3001, 1, 1, 1, 101, 1, '2025-06-20 09:30:00', 5, 50.00);
-- INSERT INTO order_line VALUES (3001, 1, 2, 3, 102, 1, '2025-06-20 09:30:00', 2, 30.00);
-- -- 更新订单号
-- UPDATE district SET d_next_o_id = 3002 WHERE d_id = 1 AND d_w_id = 1;
-- -- 更新客户账户
-- UPDATE customer SET c_balance = 920.00 WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
-- COMMIT;

-- -- 确认提交结果
-- SELECT * FROM orders WHERE o_id = 3001;
-- SELECT * FROM order_line WHERE ol_o_id = 3001 ORDER BY ol_number;
-- SELECT c_balance FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
-- SELECT d_next_o_id FROM district WHERE d_id = 1 AND d_w_id = 1;

-- -- 测试项目2：事务回滚 - 另一个新订单尝试，但取消
-- BEGIN;
-- -- 创建新订单
-- INSERT INTO orders VALUES (3002, 1, 1, 2, '2025-06-20 10:15:00', NULL, 1, 1);
-- -- 创建订单明细
-- INSERT INTO order_line VALUES (3002, 1, 1, 1, 103, 1, '2025-06-20 10:15:00', 1, 100.00);
-- -- 更新客户账户
-- UPDATE customer SET c_balance = 400.00 WHERE c_id = 2 AND c_d_id = 1 AND c_w_id = 1;
-- -- 更新订单号
-- UPDATE district SET d_next_o_id = 3003 WHERE d_id = 1 AND d_w_id = 1;
-- -- 回滚
-- ABORT;

-- -- 确认回滚结果
-- SELECT * FROM orders WHERE o_id = 3002;
-- SELECT * FROM order_line WHERE ol_o_id = 3002;
-- SELECT c_balance FROM customer WHERE c_id = 2 AND c_d_id = 1 AND c_w_id = 1;
-- SELECT d_next_o_id FROM district WHERE d_id = 1 AND d_w_id = 1;

-- -- 测试项目3：复杂查询，测试索引使用
-- SELECT c.c_id, c.c_first, c.c_last, o.o_id, o.o_entry_d FROM customer c, orders o WHERE c.c_w_id = 1 AND c.c_d_id = 1 AND o.o_w_id = c.c_w_id AND o.o_d_id = c.c_d_id AND o.o_c_id = c.c_id ORDER BY o_entry_d;

-- -- 测试项目4：日期类型查询
-- SELECT * FROM order_line WHERE ol_delivery_d = '2025-06-20 09:30:00';

-- -- 测试项目5：多表事务
-- BEGIN;
-- -- 插入新商品
-- INSERT INTO item VALUES (101, 'Keyboard', 45.99, 'Black mechanical keyboard');
-- INSERT INTO item VALUES (102, 'Mouse', 25.99, 'Wireless gaming mouse');
-- -- 更新库存
-- INSERT INTO stock VALUES (101, 1, 100, 0, 'In stock', 'Shelf A', 'Shelf B');
-- INSERT INTO stock VALUES (102, 1, 50, 0, 'In stock', 'Shelf C', 'Shelf D');
-- COMMIT;

-- -- 测试项目6：多表查询
-- SELECT i.i_name, s.s_quantity, s.s_data FROM item i, stock s WHERE i.i_id = s.s_i_id AND s.s_w_id = 1 ORDER BY i_name;

-- -- 测试项目7：索引测试（回滚）
-- BEGIN;
-- UPDATE customer SET c_balance = 2500.00 WHERE c_last = 'Johnson';
-- UPDATE item SET i_price = 49.99 WHERE i_id = 101;
-- INSERT INTO orders VALUES (3003, 2, 1, 3, '2025-06-20 14:00:00', NULL, 1, 1);
-- ABORT;

-- -- 验证索引回滚
-- SELECT c_balance FROM customer WHERE c_last = 'Johnson';
-- SELECT i_price FROM item WHERE i_id = 101;
-- SELECT * FROM orders WHERE o_id = 3003;

-- -- 测试项目8：数据完整性检查
-- BEGIN;
-- -- 确认当前值
-- SELECT * FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
-- -- 更新数据
-- UPDATE customer SET c_ytd_payment = 500.00, c_credit = 'GC' WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
-- -- 验证更新
-- SELECT * FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
-- COMMIT;

-- -- 验证提交
-- SELECT * FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;

-- ========================================
-- 超大型SQL测试集 - 包含所有功能的全面测试
-- ========================================

-- ==================== 表定义和初始化 ====================

-- 清理所有旧表
-- DROP TABLE orders;
-- DROP TABLE customer;
-- DROP TABLE district;
-- DROP TABLE warehouse;
-- DROP TABLE item;
-- DROP TABLE stock;
-- DROP TABLE history;
-- DROP TABLE new_order;
-- DROP TABLE order_line;

-- 创建仓库表
CREATE TABLE warehouse (
    w_id INT,
    w_name CHAR(10),
    w_street_1 CHAR(20),
    w_street_2 CHAR(20),
    w_city CHAR(20),
    w_state CHAR(2),
    w_zip CHAR(9),
    w_tax FLOAT,
    w_ytd FLOAT
);

-- 创建区域表
CREATE TABLE district (
    d_id INT,
    d_w_id INT,
    d_name CHAR(10),
    d_street_1 CHAR(20),
    d_street_2 CHAR(20),
    d_city CHAR(20),
    d_state CHAR(2),
    d_zip CHAR(9),
    d_tax FLOAT,
    d_ytd FLOAT,
    d_next_o_id INT
);

-- 创建客户表
CREATE TABLE customer (
    c_id INT,
    c_d_id INT,
    c_w_id INT,
    c_first CHAR(16),
    c_middle CHAR(2),
    c_last CHAR(16),
    c_street_1 CHAR(20),
    c_street_2 CHAR(20),
    c_city CHAR(20),
    c_state CHAR(2),
    c_zip CHAR(9),
    c_phone CHAR(16),
    c_credit CHAR(3),
    c_credit_lim FLOAT,
    c_discount FLOAT,
    c_balance FLOAT,
    c_ytd_payment FLOAT,
    c_payment_cnt INT,
    c_delivery_cnt INT,
    c_data CHAR(50)
);

-- 创建订单表
CREATE TABLE orders (
    o_id INT,
    o_d_id INT,
    o_w_id INT,
    o_c_id INT,
    o_entry_d CHAR(25),
    o_carrier_id INT,
    o_ol_cnt INT,
    o_all_local INT
);

-- 创建物品表
CREATE TABLE item (
    i_id INT,
    i_im_id INT,
    i_name CHAR(24),
    i_price FLOAT,
    i_data CHAR(50)
);

-- 创建库存表
CREATE TABLE stock (
    s_i_id INT,
    s_w_id INT,
    s_quantity INT,
    s_dist_01 CHAR(24),
    s_dist_02 CHAR(24),
    s_dist_03 CHAR(24),
    s_dist_04 CHAR(24),
    s_dist_05 CHAR(24),
    s_dist_06 CHAR(24),
    s_dist_07 CHAR(24),
    s_dist_08 CHAR(24),
    s_dist_09 CHAR(24),
    s_dist_10 CHAR(24),
    s_ytd INT,
    s_order_cnt INT,
    s_remote_cnt INT,
    s_data CHAR(50)
);

-- 创建历史记录表
CREATE TABLE history (
    h_c_id INT,
    h_c_d_id INT,
    h_c_w_id INT,
    h_d_id INT,
    h_w_id INT,
    h_date CHAR(25),
    h_amount FLOAT,
    h_data CHAR(24)
);

-- 创建新订单表
CREATE TABLE new_order (
    no_o_id INT,
    no_d_id INT,
    no_w_id INT
);

-- 创建订单行表
CREATE TABLE order_line (
    ol_o_id INT,
    ol_d_id INT,
    ol_w_id INT,
    ol_number INT,
    ol_i_id INT,
    ol_supply_w_id INT,
    ol_delivery_d CHAR(25),
    ol_quantity INT,
    ol_amount FLOAT,
    ol_dist_info CHAR(24)
);

-- ==================== 索引创建 ====================

-- 为常用查询创建索引
CREATE INDEX customer(c_last, c_first);
CREATE INDEX orders(o_c_id, o_d_id, o_w_id);
CREATE INDEX item(i_name);
CREATE INDEX stock(s_quantity);

-- ==================== 数据插入 ====================

-- 插入仓库数据
INSERT INTO warehouse VALUES (1, 'Warehouse1', '12345 Main St', 'Suite 100', 'Boston', 'MA', '12345', 0.05, 10000.00);
INSERT INTO warehouse VALUES (2, 'Warehouse2', '23456 First Ave', 'Floor 2', 'Chicago', 'IL', '23456', 0.06, 12000.00);
INSERT INTO warehouse VALUES (3, 'Warehouse3', '34567 Second Blvd', 'Unit 300', 'Dallas', 'TX', '34567', 0.055, 11000.00);

-- 插入区域数据
INSERT INTO district VALUES (1, 1, 'District1', '12345 Park St', 'Suite 101', 'Boston', 'MA', '12345', 0.10, 5000.00, 3001);
INSERT INTO district VALUES (2, 1, 'District2', '12346 Park St', 'Suite 201', 'Boston', 'MA', '12345', 0.11, 5100.00, 3002);
INSERT INTO district VALUES (1, 2, 'District1', '23456 Oak Ave', 'Floor 1', 'Chicago', 'IL', '23456', 0.12, 5200.00, 3003);
INSERT INTO district VALUES (2, 2, 'District2', '23457 Oak Ave', 'Floor 2', 'Chicago', 'IL', '23456', 0.13, 5300.00, 3004);

-- 插入客户数据
INSERT INTO customer VALUES (1, 1, 1, 'John', 'A', 'Smith', '12345 1st St', 'Apt 101', 'Boston', 'MA', '12345', '555-1234', 'GC', 5000.00, 0.10, 1000.00, 100.00, 1, 0, 'Customer data 1');
INSERT INTO customer VALUES (2, 1, 1, 'Mary', 'B', 'Johnson', '12346 1st St', 'Apt 102', 'Boston', 'MA', '12345', '555-2345', 'BC', 4000.00, 0.05, 900.00, 90.00, 2, 1, 'Customer data 2');
INSERT INTO customer VALUES (1, 2, 1, 'Robert', 'C', 'Williams', '12347 1st St', 'Apt 201', 'Boston', 'MA', '12345', '555-3456', 'GC', 4500.00, 0.08, 950.00, 95.00, 3, 1, 'Customer data 3');
INSERT INTO customer VALUES (1, 1, 2, 'Patricia', 'D', 'Brown', '23456 2nd Ave', 'Suite 101', 'Chicago', 'IL', '23456', '555-4567', 'GC', 5500.00, 0.12, 1100.00, 110.00, 4, 2, 'Customer data 4');
INSERT INTO customer VALUES (2, 1, 2, 'Michael', 'E', 'Davis', '23457 2nd Ave', 'Suite 102', 'Chicago', 'IL', '23456', '555-5678', 'BC', 4200.00, 0.07, 920.00, 92.00, 2, 1, 'Customer data 5');

-- 插入订单数据
INSERT INTO orders VALUES (3001, 1, 1, 1, '2023-01-15 10:30:00', 1, 2, 1);
INSERT INTO orders VALUES (3002, 1, 1, 2, '2023-01-16 11:45:00', 1, 3, 1);
INSERT INTO orders VALUES (3003, 2, 1, 1, '2023-01-17 09:15:00', 2, 1, 1);
INSERT INTO orders VALUES (3004, 1, 2, 1, '2023-01-18 14:20:00', 2, 4, 1);
INSERT INTO orders VALUES (3005, 1, 2, 2, '2023-01-19 16:35:00', 3, 2, 1);

-- 插入物品数据
INSERT INTO item VALUES (1, 101, 'Keyboard', 25.99, 'Standard keyboard');
INSERT INTO item VALUES (2, 102, 'Mouse', 15.99, 'Optical mouse');
INSERT INTO item VALUES (3, 103, 'Monitor', 149.99, '24-inch LCD monitor');
INSERT INTO item VALUES (4, 104, 'Printer', 89.99, 'Color inkjet printer');
INSERT INTO item VALUES (5, 105, 'Scanner', 79.99, 'Document scanner');
INSERT INTO item VALUES (6, 106, 'Hard Drive', 59.99, '1TB HDD');
INSERT INTO item VALUES (7, 107, 'SSD', 99.99, '500GB SSD');
INSERT INTO item VALUES (8, 108, 'RAM', 45.99, '8GB DDR4');
INSERT INTO item VALUES (9, 109, 'Graphics Card', 199.99, 'Mid-range GPU');
INSERT INTO item VALUES (10, 110, 'Motherboard', 129.99, 'ATX motherboard');

-- 插入库存数据
INSERT INTO stock VALUES (1, 1, 100, 'Dist01_1_1', 'Dist02_1_1', 'Dist03_1_1', 'Dist04_1_1', 'Dist05_1_1', 'Dist06_1_1', 'Dist07_1_1', 'Dist08_1_1', 'Dist09_1_1', 'Dist10_1_1', 10, 1, 0, 'Stock data 1_1');
INSERT INTO stock VALUES (2, 1, 150, 'Dist01_2_1', 'Dist02_2_1', 'Dist03_2_1', 'Dist04_2_1', 'Dist05_2_1', 'Dist06_2_1', 'Dist07_2_1', 'Dist08_2_1', 'Dist09_2_1', 'Dist10_2_1', 15, 2, 0, 'Stock data 2_1');
INSERT INTO stock VALUES (3, 1, 200, 'Dist01_3_1', 'Dist02_3_1', 'Dist03_3_1', 'Dist04_3_1', 'Dist05_3_1', 'Dist06_3_1', 'Dist07_3_1', 'Dist08_3_1', 'Dist09_3_1', 'Dist10_3_1', 20, 3, 1, 'Stock data 3_1');
INSERT INTO stock VALUES (4, 1, 50, 'Dist01_4_1', 'Dist02_4_1', 'Dist03_4_1', 'Dist04_4_1', 'Dist05_4_1', 'Dist06_4_1', 'Dist07_4_1', 'Dist08_4_1', 'Dist09_4_1', 'Dist10_4_1', 5, 1, 0, 'Stock data 4_1');
INSERT INTO stock VALUES (5, 1, 75, 'Dist01_5_1', 'Dist02_5_1', 'Dist03_5_1', 'Dist04_5_1', 'Dist05_5_1', 'Dist06_5_1', 'Dist07_5_1', 'Dist08_5_1', 'Dist09_5_1', 'Dist10_5_1', 8, 2, 1, 'Stock data 5_1');
INSERT INTO stock VALUES (1, 2, 120, 'Dist01_1_2', 'Dist02_1_2', 'Dist03_1_2', 'Dist04_1_2', 'Dist05_1_2', 'Dist06_1_2', 'Dist07_1_2', 'Dist08_1_2', 'Dist09_1_2', 'Dist10_1_2', 12, 2, 0, 'Stock data 1_2');
INSERT INTO stock VALUES (2, 2, 180, 'Dist01_2_2', 'Dist02_2_2', 'Dist03_2_2', 'Dist04_2_2', 'Dist05_2_2', 'Dist06_2_2', 'Dist07_2_2', 'Dist08_2_2', 'Dist09_2_2', 'Dist10_2_2', 18, 3, 1, 'Stock data 2_2');
INSERT INTO stock VALUES (3, 2, 90, 'Dist01_3_2', 'Dist02_3_2', 'Dist03_3_2', 'Dist04_3_2', 'Dist05_3_2', 'Dist06_3_2', 'Dist07_3_2', 'Dist08_3_2', 'Dist09_3_2', 'Dist10_3_2', 9, 1, 0, 'Stock data 3_2');

-- 插入订单行数据
INSERT INTO order_line VALUES (3001, 1, 1, 1, 1, 1, '2023-01-15 10:30:00', 2, 51.98, 'Dist info 3001_1');
INSERT INTO order_line VALUES (3001, 1, 1, 2, 2, 1, '2023-01-15 10:30:00', 1, 15.99, 'Dist info 3001_2');
INSERT INTO order_line VALUES (3002, 1, 1, 1, 3, 1, '2023-01-16 11:45:00', 1, 149.99, 'Dist info 3002_1');
INSERT INTO order_line VALUES (3002, 1, 1, 2, 4, 1, '2023-01-16 11:45:00', 1, 89.99, 'Dist info 3002_2');
INSERT INTO order_line VALUES (3002, 1, 1, 3, 5, 1, '2023-01-16 11:45:00', 2, 159.98, 'Dist info 3002_3');

-- 插入历史记录
INSERT INTO history VALUES (1, 1, 1, 1, 1, '2023-01-15 10:30:00', 67.97, 'Payment for order 3001');
INSERT INTO history VALUES (2, 1, 1, 1, 1, '2023-01-16 11:45:00', 399.96, 'Payment for order 3002');
INSERT INTO history VALUES (1, 2, 1, 2, 1, '2023-01-17 09:15:00', 25.99, 'Payment for order 3003');

-- 插入新订单
INSERT INTO new_order VALUES (3001, 1, 1);
INSERT INTO new_order VALUES (3002, 1, 1);
INSERT INTO new_order VALUES (3003, 2, 1);
select * from warehouse;
select * from district;
select * from customer;
select * from orders;
select * from item;
select * from stock;
select * from history;
select * from new_order;
select * from order_line;
-- -- ==================== 测试基本查询 ====================

-- -- 简单选择查询
SELECT * FROM warehouse WHERE w_id = 1;
SELECT w_name, w_city, w_state FROM warehouse WHERE w_id = 2;
SELECT * FROM customer WHERE c_balance > 1000.00;

-- 投影查询
SELECT d_name, d_city, d_state FROM district WHERE d_w_id = 1;
SELECT c_first, c_last, c_balance FROM customer WHERE c_credit = 'GC';

-- 条件查询
SELECT * FROM item WHERE i_price < 50.00;
SELECT * FROM stock WHERE s_quantity > 100 AND s_w_id = 1;
SELECT * FROM customer WHERE c_balance > 900.00 AND c_credit = 'BC';

-- 排序查询
SELECT * FROM item ORDER BY i_price DESC;
-- SELECT * FROM customer ORDER BY c_last ASC;
SELECT * FROM stock WHERE s_w_id = 1 ORDER BY s_quantity ASC;

-- 聚合查询
SELECT COUNT(*) FROM orders;
SELECT SUM(c_balance) FROM customer WHERE c_w_id = 1;
SELECT MAX(i_price) FROM item;
SELECT MIN(s_quantity) FROM stock WHERE s_w_id = 1;
SELECT AVG(c_balance) FROM customer WHERE c_w_id = 1 AND c_d_id = 1;

-- 分组查询
SELECT c_w_id, COUNT(*) FROM customer GROUP BY c_w_id;
SELECT o_w_id, o_d_id, COUNT(*) FROM orders GROUP BY o_w_id, o_d_id;
SELECT s_w_id, AVG(s_quantity) FROM stock GROUP BY s_w_id;

-- ==================== 测试复杂查询 ====================

-- 多表连接
SELECT c.c_first, c.c_last, o.o_id, o.o_entry_d FROM customer c, orders o WHERE c.c_id = o.o_c_id AND c.c_d_id = o.o_d_id AND c.c_w_id = o.o_w_id;

-- 三表连接
SELECT c.c_first, c.c_last, o.o_id, ol.ol_amount 
FROM customer c, orders o, order_line ol 
WHERE c.c_id = o.o_c_id AND c.c_d_id = o.o_d_id AND c.c_w_id = o.o_w_id 
  AND o.o_id = ol.ol_o_id AND o.o_d_id = ol.ol_d_id AND o.o_w_id = ol.ol_w_id;

-- 连接+条件+排序
SELECT i.i_name, s.s_quantity, s.s_ytd FROM item i, stock s WHERE i.i_id = s.s_i_id AND s.s_w_id = 1 AND s.s_quantity > 50 ORDER BY s.s_quantity DESC;

-- 连接+聚合
SELECT w.w_name, COUNT(o.o_id) as order_count 
FROM warehouse w, district d, orders o 
WHERE w.w_id = d.d_w_id AND d.d_id = o.o_d_id AND d.d_w_id = o.o_w_id 
GROUP BY w.w_id, w.w_name;

-- 连接+聚合+条件
SELECT c.c_last, COUNT(o.o_id) as order_count, SUM(o.o_ol_cnt) as total_items 
FROM customer c, orders o 
WHERE c.c_id = o.o_c_id AND c.c_d_id = o.o_d_id AND c.c_w_id = o.o_w_id AND c.c_credit = 'GC' 
GROUP BY c.c_id, c.c_last 
HAVING COUNT(o.o_id) > 0;

-- ==================== 测试更新操作 ====================

-- 开始事务
BEGIN;

-- 单条件更新
UPDATE warehouse SET w_ytd = 15000.00 WHERE w_id = 1;

-- 多条件更新
UPDATE customer SET c_balance = 1500.00, c_ytd_payment = 200.00 WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;

-- 验证更新
SELECT * FROM warehouse WHERE w_id = 1;
SELECT * FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;

-- 提交事务
COMMIT;

-- 二次验证
SELECT * FROM warehouse WHERE w_id = 1;
SELECT * FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;

-- ==================== 测试删除操作 ====================

-- 开始事务
BEGIN;

-- 单表单条件删除
DELETE FROM new_order WHERE no_o_id = 3001;

-- 多条件删除
DELETE FROM history WHERE h_c_id = 1 AND h_c_d_id = 1 AND h_c_w_id = 1;

-- 验证删除
SELECT * FROM new_order WHERE no_o_id = 3001;
SELECT * FROM history WHERE h_c_id = 1 AND h_c_d_id = 1 AND h_c_w_id = 1;

-- 提交事务
COMMIT;

-- 二次验证
SELECT * FROM new_order WHERE no_o_id = 3001;
SELECT * FROM history WHERE h_c_id = 1 AND h_c_d_id = 1 AND h_c_w_id = 1;

-- ==================== 测试事务操作 ====================

-- 测试事务提交
BEGIN;
UPDATE stock SET s_quantity = s_quantity - 10 WHERE s_i_id = 1 AND s_w_id = 1;
SELECT s_quantity FROM stock WHERE s_i_id = 1 AND s_w_id = 1;
COMMIT;
SELECT s_quantity FROM stock WHERE s_i_id = 1 AND s_w_id = 1;

-- 测试事务回滚
BEGIN;
UPDATE stock SET s_quantity = s_quantity - 20 WHERE s_i_id = 2 AND s_w_id = 1;
SELECT s_quantity FROM stock WHERE s_i_id = 2 AND s_w_id = 1;
ABORT;
SELECT s_quantity FROM stock WHERE s_i_id = 2 AND s_w_id = 1;

-- ==================== 测试索引操作 ====================

-- 创建和删除索引
CREATE INDEX stock(s_w_id, s_i_id);
CREATE INDEX customer(c_balance);
DROP INDEX stock(s_quantity);

-- 查询检验索引是否有效
SELECT * FROM customer WHERE c_balance > 1000.00;
SELECT * FROM customer WHERE c_last = 'Smith';

-- ==================== 高级更新操作 ====================

-- 批量更新
BEGIN;
UPDATE item SET i_price = i_price * 1.10 WHERE i_price < 50.00;
UPDATE item SET i_price = i_price * 1.05 WHERE i_price >= 50.00 AND i_price < 100.00;
UPDATE item SET i_price = i_price * 1.03 WHERE i_price >= 100.00;
COMMIT;

-- 查看更新结果
SELECT i_id, i_name, i_price FROM item ORDER BY i_price;

-- ==================== 压力测试 - 大量数据插入 ====================

-- 批量插入物品 (11-30)
BEGIN;
INSERT INTO item VALUES (11, 111, 'USB Cable', 9.99, 'USB-C cable');
INSERT INTO item VALUES (12, 112, 'HDMI Cable', 14.99, 'HDMI 2.1 cable');
INSERT INTO item VALUES (13, 113, 'Webcam', 39.99, 'HD webcam');
INSERT INTO item VALUES (14, 114, 'Headphones', 49.99, 'Wired headphones');
INSERT INTO item VALUES (15, 115, 'Speakers', 69.99, 'Desktop speakers');
INSERT INTO item VALUES (16, 116, 'Microphone', 59.99, 'Condenser microphone');
INSERT INTO item VALUES (17, 117, 'Power Bank', 24.99, '10000mAh capacity');
INSERT INTO item VALUES (18, 118, 'Router', 79.99, 'Wireless router');
INSERT INTO item VALUES (19, 119, 'External HDD', 89.99, '2TB external hard drive');
INSERT INTO item VALUES (20, 120, 'Laptop Bag', 29.99, 'Black laptop bag');
COMMIT;

-- 批量插入库存 (新物品在仓库1)
BEGIN;
INSERT INTO stock VALUES (11, 1, 50, 'D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 5, 1, 0, 'Stock 11_1');
INSERT INTO stock VALUES (12, 1, 60, 'D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 6, 1, 0, 'Stock 12_1');
INSERT INTO stock VALUES (13, 1, 40, 'D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 4, 1, 0, 'Stock 13_1');
INSERT INTO stock VALUES (14, 1, 30, 'D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 3, 1, 0, 'Stock 14_1');
INSERT INTO stock VALUES (15, 1, 25, 'D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 2, 1, 0, 'Stock 15_1');
INSERT INTO stock VALUES (16, 1, 20, 'D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 2, 1, 0, 'Stock 16_1');
INSERT INTO stock VALUES (17, 1, 35, 'D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 3, 1, 0, 'Stock 17_1');
INSERT INTO stock VALUES (18, 1, 15, 'D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 1, 1, 0, 'Stock 18_1');
INSERT INTO stock VALUES (19, 1, 10, 'D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 1, 1, 0, 'Stock 19_1');
INSERT INTO stock VALUES (20, 1, 45, 'D01', 'D02', 'D03', 'D04', 'D05', 'D06', 'D07', 'D08', 'D09', 'D10', 4, 1, 0, 'Stock 20_1');
COMMIT;

-- 验证插入
SELECT COUNT(*) FROM item;
SELECT COUNT(*) FROM stock WHERE s_w_id = 1;

-- ==================== 最终综合测试 ====================

-- 复杂多表查询 - 查找所有顾客订单及其包含的产品
SELECT c.c_first, c.c_last, o.o_id, o.o_entry_d, i.i_name, ol.ol_quantity, ol.ol_amount
FROM customer c, orders o, order_line ol, item i
WHERE c.c_id = o.o_c_id AND c.c_d_id = o.o_d_id AND c.c_w_id = o.o_w_id
  AND o.o_id = ol.ol_o_id AND o.o_d_id = ol.ol_d_id AND o.o_w_id = ol.ol_w_id
  AND ol.ol_i_id = i.i_id
ORDER BY c.c_last;

-- 大型事务 - 模拟订单处理
BEGIN;

-- 1. 减少库存
UPDATE stock SET s_quantity = s_quantity - 5 WHERE s_i_id = 1 AND s_w_id = 1;
UPDATE stock SET s_quantity = s_quantity - 3 WHERE s_i_id = 2 AND s_w_id = 1;
UPDATE stock SET s_quantity = s_quantity - 2 WHERE s_i_id = 3 AND s_w_id = 1;

-- 2. 创建新订单
INSERT INTO orders VALUES (3006, 1, 1, 1, '2023-01-20 13:45:00', 1, 3, 1);

-- 3. 添加订单行
INSERT INTO order_line VALUES (3006, 1, 1, 1, 1, 1, '2023-01-20 13:45:00', 5, 129.95, 'Dist info 3006_1');
INSERT INTO order_line VALUES (3006, 1, 1, 2, 2, 1, '2023-01-20 13:45:00', 3, 47.97, 'Dist info 3006_2');
INSERT INTO order_line VALUES (3006, 1, 1, 3, 3, 1, '2023-01-20 13:45:00', 2, 299.98, 'Dist info 3006_3');

-- 4. 更新客户余额
UPDATE customer SET c_balance = c_balance - 477.90, c_delivery_cnt = c_delivery_cnt + 1 
WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;

-- 5. 添加到新订单表
INSERT INTO new_order VALUES (3006, 1, 1);

-- 6. 添加历史记录
INSERT INTO history VALUES (1, 1, 1, 1, 1, '2023-01-20 13:45:00', 477.90, 'Payment for order 3006');

COMMIT;

-- 验证事务结果
SELECT * FROM orders WHERE o_id = 3006;
SELECT * FROM order_line WHERE ol_o_id = 3006;
SELECT * FROM stock WHERE s_i_id IN (1, 2, 3) AND s_w_id = 1;
SELECT * FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
SELECT * FROM new_order WHERE no_o_id = 3006;
SELECT * FROM history WHERE h_data = 'Payment for order 3006';

-- ==================== 收尾工作 ====================
-- 显示最终的表数据统计
SELECT 'warehouse' as table_name, COUNT(*) as row_count FROM warehouse
UNION ALL
SELECT 'district', COUNT(*) FROM district
UNION ALL
SELECT 'customer', COUNT(*) FROM customer
UNION ALL
SELECT 'orders', COUNT(*) FROM orders
UNION ALL
SELECT 'item', COUNT(*) FROM item
UNION ALL
SELECT 'stock', COUNT(*) FROM stock
UNION ALL
SELECT 'order_line', COUNT(*) FROM order_line
UNION ALL
SELECT 'history', COUNT(*) FROM history
UNION ALL
SELECT 'new_order', COUNT(*) FROM new_order;

-- 所有测试完成