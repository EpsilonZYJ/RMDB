-- TPC-C NewOrder事务风格事务管理测试用例
-- 涉及索引、date类型、事务提交与回滚

-- 1. 创建表结构，包含索引和date类型
DROP TABLE warehouse;
DROP TABLE district;
DROP TABLE customer;
DROP TABLE orders;
DROP TABLE order_line;

CREATE TABLE warehouse (w_id int,w_name CHAR(16),w_created DATE);
CREATE INDEX warehouse(w_name);

CREATE TABLE district (d_id int,d_w_id INT,d_next_o_id INT,d_created DATE);
CREATE INDEX district(d_w_id);
CREATE TABLE customer (c_id Int,c_d_id INT,c_w_id INT,c_name CHAR(16),c_balance INT,c_since DATE);
CREATE INDEX customer(c_name);

CREATE TABLE orders (o_id INT,o_d_id INT,o_w_id INT,o_c_id INT,o_entry_d DATE);
CREATE INDEX orders(o_c_id);
CREATE TABLE order_line (ol_o_id INT,ol_number INT,ol_i_id INT,ol_amount INT,ol_delivery_d DATE);
CREATE INDEX order_line(ol_i_id);

-- 2. 插入初始数据
INSERT INTO warehouse VALUES (1, 'WH1', '2025-06-01 14:00:00');
INSERT INTO district VALUES (1, 1, 3001, '2025-06-01 14:00:00');
INSERT INTO customer VALUES (1, 1, 1, 'Alice', 1000, '2025-06-01 14:00:00');
INSERT INTO customer VALUES (2, 1, 1, 'Bob', 800, '2025-06-01 14:00:00');

-- 3. NewOrder事务：正常提交
BEGIN;
-- 读取district，获取下一个订单号
SELECT d_next_o_id FROM district WHERE d_id = 1;
-- 插入新订单
INSERT INTO orders VALUES (3001, 1, 1, 1, '2025-06-19 14:00:00');
-- 插入order_line
INSERT INTO order_line VALUES (3001, 1, 101, 5, '2025-06-19 14:00:00');
INSERT INTO order_line VALUES (3001, 2, 102, 3, '2025-06-19 14:00:00');
-- district订单号+1
UPDATE district SET d_next_o_id = 1 WHERE d_id = 1;
COMMIT;

-- 4. NewOrder事务：回滚
BEGIN;
INSERT INTO orders VALUES (3002, 1, 1, 2, '2025-06-19 14:00:00');
INSERT INTO order_line VALUES (3002, 1, 103, 2,'2025-06-19 14:00:00');
UPDATE customer SET c_balance = 200 WHERE c_id = 2;
-- 故意回滚


-- 5. 检查提交与回滚效果
SELECT * FROM orders ORDER BY o_id;
SELECT * FROM order_line ORDER BY ol_o_id;
SELECT * FROM customer ORDER BY c_id;
SELECT d_next_o_id FROM district WHERE d_id = 1;

-- 6. 索引与date类型相关操作
-- 查询索引字段
SELECT * FROM warehouse WHERE w_name = 'WH1';
SELECT * FROM customer WHERE c_name = 'Alice';
-- 查询date类型
SELECT * FROM orders WHERE o_entry_d =  '2025-06-19 14:00:00';
SELECT * FROM order_line WHERE ol_delivery_d =  '2025-06-19 14:00:00';

