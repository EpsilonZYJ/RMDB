-- 清理旧表
DROP TABLE warehouse;
DROP TABLE district;
DROP TABLE customer;
DROP TABLE orders;
DROP TABLE order_line;
DROP TABLE item;
DROP TABLE stock;

-- 创建表结构（包含int、float、string、date类型）
CREATE TABLE warehouse (w_id INT,w_name CHAR(16),w_street CHAR(32),w_city CHAR(16),w_tax FLOAT,w_ytd FLOAT,w_created DATE);
CREATE INDEX warehouse(w_id);

CREATE TABLE district (d_id INT,d_w_id INT,d_name CHAR(16),d_tax FLOAT,d_next_o_id INT,d_created DATE);
CREATE INDEX district(d_w_id, d_id);

CREATE TABLE customer (c_id INT,c_d_id INT,c_w_id INT,c_first CHAR(16),c_last CHAR(16),c_credit CHAR(4),c_balance FLOAT,c_ytd_payment FLOAT,c_since DATE);
CREATE INDEX customer(c_d_id, c_id);
CREATE INDEX customer(c_last);

CREATE TABLE orders (o_id INT,o_d_id INT,o_w_id INT,o_c_id INT,o_entry_d DATE,o_carrier_id INT,o_ol_cnt INT,o_all_local INT);
CREATE INDEX orders(o_w_id, o_d_id, o_id);
CREATE INDEX orders(o_w_id, o_d_id, o_c_id);

CREATE TABLE order_line (ol_o_id INT,ol_d_id INT,ol_w_id INT,ol_number INT,ol_i_id INT,ol_supply_w_id INT,ol_delivery_d DATE,ol_quantity INT,ol_amount FLOAT);
CREATE INDEX order_line(ol_w_id, ol_d_id, ol_o_id);

CREATE TABLE item (i_id INT,i_name CHAR(32),i_price FLOAT,i_data CHAR(64));
CREATE INDEX item(i_id);

CREATE TABLE stock (s_i_id INT,s_w_id INT,s_quantity INT,s_ytd INT,s_data CHAR(64),s_dist_01 CHAR(24),s_dist_02 CHAR(24));
CREATE INDEX stock(s_w_id, s_i_id);

-- 插入初始数据
INSERT INTO warehouse VALUES (1, 'Warehouse-001', '123 Main St', 'Seattle', 0.05, 100000.00, '2023-01-01 08:00:00');
INSERT INTO warehouse VALUES (2, 'Warehouse-002', '456 Elm St', 'Portland', 0.06, 200000.00, '2023-01-02 09:30:00');

INSERT INTO district VALUES (1, 1, 'North', 0.05, 3001, '2023-01-03 10:15:00');
INSERT INTO district VALUES (2, 1, 'South', 0.05, 3001, '2023-01-03 10:20:00');
INSERT INTO district VALUES (1, 2, 'East', 0.06, 3001, '2023-01-04 11:00:00');

INSERT INTO customer VALUES (1, 1, 1, 'Alice', 'Johnson', 'GC', 1000.00, 0.00, '2023-01-05 14:30:00');
INSERT INTO customer VALUES (2, 1, 1, 'Bob', 'Smith', 'BC', 500.00, 0.00, '2023-01-06 09:15:00');
INSERT INTO customer VALUES (3, 2, 1, 'Carol', 'Davis', 'GC', 1500.00, 0.00, '2023-01-07 16:45:00');
INSERT INTO customer VALUES (1, 3, 2, 'David', 'Miller', 'GC', 2000.00, 0.00, '2023-01-08 11:20:00');

-- 测试项目1：基本事务提交 - 新订单创建
BEGIN;
-- 创建新订单
INSERT INTO orders VALUES (3001, 1, 1, 1, '2025-06-20 09:30:00', 0, 2, 1);
-- 创建订单明细
INSERT INTO order_line VALUES (3001, 1, 1, 1, 101, 1, '2025-06-20 09:30:00', 5, 50.00);
INSERT INTO order_line VALUES (3001, 1, 2, 3, 102, 1, '2025-06-20 09:30:00', 2, 30.00);
-- 更新订单号
UPDATE district SET d_next_o_id = 3002 WHERE d_id = 1 AND d_w_id = 1;
-- 更新客户账户
UPDATE customer SET c_balance = 920.00 WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
COMMIT;

-- 确认提交结果
SELECT * FROM orders WHERE o_id = 3001;
SELECT * FROM order_line WHERE ol_o_id = 3001 ORDER BY ol_number;
SELECT c_balance FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
SELECT d_next_o_id FROM district WHERE d_id = 1 AND d_w_id = 1;

-- 测试项目2：事务回滚 - 另一个新订单尝试，但取消
BEGIN;
-- 创建新订单
INSERT INTO orders VALUES (3002, 1, 1, 2, '2025-06-20 10:15:00', NULL, 1, 1);
-- 创建订单明细
INSERT INTO order_line VALUES (3002, 1, 1, 1, 103, 1, '2025-06-20 10:15:00', 1, 100.00);
-- 更新客户账户
UPDATE customer SET c_balance = 400.00 WHERE c_id = 2 AND c_d_id = 1 AND c_w_id = 1;
-- 更新订单号
UPDATE district SET d_next_o_id = 3003 WHERE d_id = 1 AND d_w_id = 1;
-- 回滚
ABORT;

-- 确认回滚结果
SELECT * FROM orders WHERE o_id = 3002;
SELECT * FROM order_line WHERE ol_o_id = 3002;
SELECT c_balance FROM customer WHERE c_id = 2 AND c_d_id = 1 AND c_w_id = 1;
SELECT d_next_o_id FROM district WHERE d_id = 1 AND d_w_id = 1;

-- 测试项目3：复杂查询，测试索引使用
SELECT c.c_id, c.c_first, c.c_last, o.o_id, o.o_entry_d FROM customer c, orders o WHERE c.c_w_id = 1 AND c.c_d_id = 1 AND o.o_w_id = c.c_w_id AND o.o_d_id = c.c_d_id AND o.o_c_id = c.c_id ORDER BY o_entry_d;

-- 测试项目4：日期类型查询
SELECT * FROM order_line WHERE ol_delivery_d = '2025-06-20 09:30:00';

-- 测试项目5：多表事务
BEGIN;
-- 插入新商品
INSERT INTO item VALUES (101, 'Keyboard', 45.99, 'Black mechanical keyboard');
INSERT INTO item VALUES (102, 'Mouse', 25.99, 'Wireless gaming mouse');
-- 更新库存
INSERT INTO stock VALUES (101, 1, 100, 0, 'In stock', 'Shelf A', 'Shelf B');
INSERT INTO stock VALUES (102, 1, 50, 0, 'In stock', 'Shelf C', 'Shelf D');
COMMIT;

-- 测试项目6：多表查询
SELECT i.i_name, s.s_quantity, s.s_data FROM item i, stock s WHERE i.i_id = s.s_i_id AND s.s_w_id = 1 ORDER BY i_name;

-- 测试项目7：索引测试（回滚）
BEGIN;
UPDATE customer SET c_balance = 2500.00 WHERE c_last = 'Johnson';
UPDATE item SET i_price = 49.99 WHERE i_id = 101;
INSERT INTO orders VALUES (3003, 2, 1, 3, '2025-06-20 14:00:00', NULL, 1, 1);
ABORT;

-- 验证索引回滚
SELECT c_balance FROM customer WHERE c_last = 'Johnson';
SELECT i_price FROM item WHERE i_id = 101;
SELECT * FROM orders WHERE o_id = 3003;

-- 测试项目8：数据完整性检查
BEGIN;
-- 确认当前值
SELECT * FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
-- 更新数据
UPDATE customer SET c_ytd_payment = 500.00, c_credit = 'GC' WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
-- 验证更新
SELECT * FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;
COMMIT;

-- 验证提交
SELECT * FROM customer WHERE c_id = 1 AND c_d_id = 1 AND c_w_id = 1;