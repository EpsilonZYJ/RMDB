-- 最小化测试，验证事务基本功能
CREATE TABLE test_txn (id INT, name CHAR(20));

-- 测试1: 简单插入提交
BEGIN;
INSERT INTO test_txn VALUES (1, 'test1');
COMMIT;
SELECT * FROM test_txn;

-- 测试2: 简单回滚
BEGIN;
INSERT INTO test_txn VALUES (2, 'test2');
ABORT;
SELECT * FROM test_txn;

CREATE TABLE index_test (id INT,name CHAR(10), score FLOAT);
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