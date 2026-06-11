-- 创建简单表
CREATE TABLE simple_test (id INT, name CHAR(10));

-- TEST 1.1: 简单提交
BEGIN;
INSERT INTO simple_test VALUES (1, 'test1');
COMMIT;
SELECT * FROM simple_test; -- 应该有一条记录

-- TEST 1.2: 简单回滚
BEGIN;
INSERT INTO simple_test VALUES (2, 'test2');
ABORT;
SELECT * FROM simple_test; -- 应该仍然只有一条记录

-- TEST 1.3: 空事务提交
BEGIN;
COMMIT;
SELECT * FROM simple_test; -- 应该仍然只有一条记录

-- TEST 1.4: 空事务回滚
BEGIN;
ABORT;
SELECT * FROM simple_test; -- 应该仍然只有一条记录

-- TEST 2.1: 多插入提交
BEGIN;
INSERT INTO simple_test VALUES (3, 'test3');
INSERT INTO simple_test VALUES (4, 'test4');
INSERT INTO simple_test VALUES (5, 'test5');
COMMIT;
SELECT * FROM simple_test; -- 应该有4条记录

-- TEST 2.2: 多插入回滚
BEGIN;
INSERT INTO simple_test VALUES (6, 'test6');
INSERT INTO simple_test VALUES (7, 'test7');
ABORT;
SELECT * FROM simple_test; -- 应该仍然只有4条记录

-- TEST 2.3: 更新提交
BEGIN;
UPDATE simple_test SET name = 'updated3' WHERE id = 3;
UPDATE simple_test SET name = 'updated4' WHERE id = 4;
COMMIT;
SELECT * FROM simple_test; -- id=3,4的记录应该被更新

-- TEST 2.4: 更新回滚
BEGIN;
UPDATE simple_test SET name = 'rolled5' WHERE id = 5;
ABORT;
SELECT * FROM simple_test; -- id=5的记录不应该更改

-- TEST 2.5: 删除提交
BEGIN;
DELETE FROM simple_test WHERE id = 4;
COMMIT;
SELECT * FROM simple_test; -- id=4的记录应该被删除

-- TEST 2.6: 删除回滚
BEGIN;
DELETE FROM simple_test WHERE id = 3;
ABORT;
SELECT * FROM simple_test; -- id=3的记录不应该被删除

-- TEST 3.1: 混合操作提交
BEGIN;
INSERT INTO simple_test VALUES (8, 'test8');
UPDATE simple_test SET name = 'updated5' WHERE id = 5;
DELETE FROM simple_test WHERE id = 1;
COMMIT;
SELECT * FROM simple_test; -- 应该有id=3,5,8的记录，id=5的name应该是'updated5'

-- TEST 3.2: 混合操作回滚
BEGIN;
INSERT INTO simple_test VALUES (9, 'test9');
UPDATE simple_test SET name = 'rolled3' WHERE id = 3;
DELETE FROM simple_test WHERE id = 5;
ABORT;
SELECT * FROM simple_test; -- 记录状态应该与TEST 3.1结束后相同

-- 创建带索引的表
CREATE TABLE index_test (id INT, name CHAR(10), score FLOAT);
CREATE INDEX index_test (id);

-- TEST 4.1: 索引表插入提交
BEGIN;
INSERT INTO index_test VALUES (1, 'index1', 90.5);
INSERT INTO index_test VALUES (2, 'index2', 85.0);
COMMIT;
SELECT * FROM index_test WHERE id = 1; -- 应该能查到id=1的记录
SELECT * FROM index_test WHERE id = 2; -- 应该能查到id=2的记录

-- TEST 4.2: 索引表插入回滚
BEGIN;
INSERT INTO index_test VALUES (3, 'index3', 92.5);
ABORT;
SELECT * FROM index_test WHERE id = 3; -- 应该查不到记录

-- TEST 4.3: 索引表更新提交
BEGIN;
UPDATE index_test SET score = 95.0 WHERE id = 1;
COMMIT;
SELECT * FROM index_test WHERE id = 1; -- score应该是95.0

-- TEST 4.4: 索引表更新索引列提交
BEGIN;
UPDATE index_test SET id = 10 WHERE id = 2;
COMMIT;
SELECT * FROM index_test WHERE id = 2; -- 应该查不到记录
SELECT * FROM index_test WHERE id = 10; -- 应该能查到记录

-- TEST 4.5: 索引表删除提交
BEGIN;
DELETE FROM index_test WHERE id = 10;
COMMIT;
SELECT * FROM index_test WHERE id = 10; -- 应该查不到记录

-- TEST 4.6: 索引表混合操作回滚
BEGIN;
INSERT INTO index_test VALUES (20, 'index20', 88.0);
UPDATE index_test SET id = 30 WHERE id = 1;
DELETE FROM index_test WHERE id = 30;
ABORT;
SELECT * FROM index_test WHERE id = 1; -- 应该能查到记录
SELECT * FROM index_test WHERE id = 20; -- 应该查不到记录
SELECT * FROM index_test WHERE id = 30; -- 应该查不到记录

-- 创建包含DATE类型的表
CREATE TABLE date_test (id INT, event_name CHAR(20), event_date DATE);

-- TEST 5.1: DATE类型插入提交
BEGIN;
INSERT INTO date_test VALUES (1, 'Meeting', '2025-06-15 09:00:00');
INSERT INTO date_test VALUES (2, 'Conference', '2025-06-20 13:30:00');
COMMIT;
SELECT * FROM date_test; -- 应该有两条记录

-- TEST 5.2: DATE类型查询
SELECT * FROM date_test WHERE event_date = '2025-06-15 09:00:00'; -- 应该查到id=1的记录

-- TEST 5.3: 创建DATE类型索引
CREATE INDEX date_test (event_date);

-- TEST 5.4: DATE类型索引查询
SELECT * FROM date_test WHERE event_date = '2025-06-20 13:30:00'; -- 应该查到id=2的记录

-- TEST 5.5: DATE类型更新提交
BEGIN;
UPDATE date_test SET event_date = '2025-07-01 10:00:00' WHERE id = 1;
COMMIT;
SELECT * FROM date_test WHERE event_date = '2025-07-01 10:00:00'; -- 应该查到id=1的记录

-- TEST 5.6: DATE类型更新回滚
BEGIN;
UPDATE date_test SET event_date = '2025-07-10 11:00:00' WHERE id = 2;
ABORT;
SELECT * FROM date_test WHERE event_date = '2025-06-20 13:30:00'; -- 应该查到id=2的记录
SELECT * FROM date_test WHERE event_date = '2025-07-10 11:00:00'; -- 应该查不到记录