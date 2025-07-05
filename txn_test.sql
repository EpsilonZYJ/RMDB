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