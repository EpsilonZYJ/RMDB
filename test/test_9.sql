-- 创建表结构
create table t1 (id int, name char(20), val int);
create table t2 (id int, info char(30));
create index icreate (id int, name char(20), val int);

-- 1. 基本插入+提交+崩溃
begin;
insert into t1 values(1, 'a', 100);
insert into t1 values(2, 'b', 200);
commit;
crash
select * from t1 order by id; -- 应有1,2

-- 2. 插入+未提交+崩溃
begin;
insert into t1 values(3, 'c', 300);
crash
select * from t1 order by id; -- 3不应出现

-- 3. 插入+提交+检查点+崩溃
begin;
insert into t1 values(4, 'd', 400);
commit;
create static_checkpoint;
crash
select * from t1 order by id; -- 应有1,2,4

-- 4. 插入+未提交+检查点+崩溃
begin;
insert into t1 values(5, 'e', 500);
create static_checkpoint;
crash
select * from t1 order by id; -- 5不应出现

-- 5. 插入+提交+检查点+更新+提交+崩溃
begin;
insert into t1 values(6, 'f', 600);
commit;
create static_checkpoint;
begin;
update t1 set val = 666 where id = 6;
commit;
crash
select * from t1 where id = 6; -- val应为666

-- 6. 插入+提交+检查点+删除+未提交+崩溃
begin;
insert into t1 values(7, 'g', 700);
commit;
create static_checkpoint;
begin;
delete from t1 where id = 7;
-- 未提交
crash
select * from t1 where id = 7; -- 7应还在

-- 7. 插入+提交+检查点+删除+提交+崩溃
begin;
insert into t1 values(8, 'h', 800);
commit;
create static_checkpoint;
begin;
delete from t1 where id = 8;
commit;
crash
select * from t1 where id = 8; -- 8不应出现

-- 8. 多事务并发+部分提交+崩溃
begin; -- t1
insert into t2 values(1, 'info1');
begin; -- t2
insert into t2 values(2, 'info2');
commit; -- t2
-- t1未提交
crash
select * from t2 order by id; -- 只应有2

-- 9. DDL+插入+崩溃
create table t3 (id int, c char(10));
begin;
insert into t3 values(1, 'x');
commit;
crash
select * from t3; -- 应有1

-- 10. 索引插入+更新+崩溃
begin;
insert into t1 values(9, 'i', 900);
commit;
create static_checkpoint;
begin;
update t1 set id = 10 where id = 9;
commit;
crash
select * from t1 where id = 10; -- 应有一条
select * from t1 where id = 9;  -- 不应有

-- 11. 回滚测试
begin;
insert into t1 values(11, 'j', 1100);
rollback;
crash
select * from t1 where id = 11; -- 不应有

-- 12. 检查点后未提交多操作
begin;
insert into t1 values(12, 'k', 1200);
insert into t1 values(13, 'l', 1300);
create static_checkpoint;
insert into t1 values(14, 'm', 1400);
-- 未提交
crash
select * from t1 where id >= 12; -- 12,13,14都不应有

-- 13. 多表操作+崩溃
begin;
insert into t1 values(15, 'n', 1500);
insert into t2 values(3, 'info3');
commit;
crash
select * from t1 where id = 15; -- 应有
select * from t2 where id = 3;  -- 应有

-- 14. 检查点后DDL
create static_checkpoint;
create table t4 (id int, c char(10));
begin;
insert into t4 values(1, 'y');
commit;
crash
select * from t4; -- 应有1

-- 15. 检查点后未提交DDL
create static_checkpoint;
create table t5 (id int, c char(10));
-- 未提交插入
begin;
insert into t5 values(1, 'z');
-- 未提交
crash
select * from t5; -- 应为空或表不存在

-- 16. 复杂事务混合
begin;
insert into t1 values(16, 'o', 1600);
insert into t2 values(4, 'info4');
commit;
begin;
insert into t1 values(17, 'p', 1700);
-- 未提交
crash
select * from t1 where id = 16; -- 应有
select * from t1 where id = 17; -- 不应有
select * from t2 where id = 4;  -- 应有