-- 创建基本表结构
create table accounts (id int, name char(20), balance int);

-- 测试简单事务+崩溃
begin;
insert into accounts values(1, 'Alice', 1000);
insert into accounts values(2, 'Bob', 2000);
commit;

begin;
insert into accounts values(3, 'Charlie', 3000);
-- 模拟崩溃 (不执行commit)
crash

-- 重启后验证
select * from accounts; -- 应只有Alice和Bob的记录

-- 准备数据
create table accounts (id int, name char(20), balance int);

begin;
insert into accounts values(1, 'Alice', 1000);
insert into accounts values(2, 'Bob', 2000);
commit;

-- 创建检查点
create static_checkpoint;

begin;
insert into accounts values(3, 'Charlie', 3000);
-- 模拟崩溃
crash

-- 重启后验证
select * from accounts; -- 应只有Alice和Bob的记录


create table customers (id int, name char(20));
create table orders (id int, customer_id int, amount int);

-- 提交的事务
begin;
insert into customers values(1, 'Alice');
insert into customers values(2, 'Bob');
insert into orders values(101, 1, 500);
commit;

-- 创建检查点
create static_checkpoint;

-- 未提交事务
begin;
insert into customers values(3, 'Charlie');
insert into orders values(102, 3, 300);
-- 模拟崩溃
crash

-- 重启后验证
select * from customers; -- 应只有Alice和Bob
select * from orders;    -- 应只有订单101


create table inventory (id int, item char(20), quantity int);

-- 初始化数据
begin;
insert into inventory values(1, 'Apple', 100);
insert into inventory values(2, 'Banana', 200);
insert into inventory values(3, 'Orange', 150);
commit;

-- 创建检查点
create static_checkpoint;

-- 提交的更新
begin;
update inventory set quantity = 90 where id = 1;
commit;

-- 未提交的删除
begin;
delete from inventory where id = 3;
-- 模拟崩溃
crash

-- 重启后验证
select * from inventory; -- Apple应为90，Orange应仍存在

create table indexed_data (id int, name char(20), value int);
create index idx_id on indexed_data(id);
create index idx_name on indexed_data(name);

-- 插入数据
begin;
insert into indexed_data values(1, 'one', 100);
insert into indexed_data values(2, 'two', 200);
insert into indexed_data values(3, 'three', 300);
commit;

-- 创建检查点
create static_checkpoint;

-- 更新索引列
begin;
update indexed_data set name = 'ONE' where id = 1;
commit;

-- 未提交的索引列操作
begin;
insert into indexed_data values(4, 'four', 400);
update indexed_data set id = 5 where id = 3;
-- 模拟崩溃
crash

-- 重启后验证
select * from indexed_data order by id;
-- 通过索引查询验证
select * from indexed_data where id = 1;
select * from indexed_data where name = 'ONE';


create table accounts (id int, name char(20), balance int);

-- 初始化
begin;
insert into accounts values(1, 'Alice', 1000);
insert into accounts values(2, 'Bob', 2000);
commit;

-- 创建检查点
create static_checkpoint;

-- 多事务测试
begin; -- 事务1
update accounts set balance = 1500 where id = 1;

-- 开启新连接，执行事务2
-- 在新连接中:
begin; -- 事务2
update accounts set balance = 2500 where id = 2;
commit; -- 提交事务2

-- 返回原连接，不提交事务1
-- 模拟崩溃
crash

-- 重启后验证
select * from accounts; -- Alice应为1000，Bob应为2500

--0
create table checkpoint_test (id int, val char(20));
--1
-- 初始数据
begin;
insert into checkpoint_test values(1, 'initial');
commit;
--2
-- 第一个检查点
create static_checkpoint;
--3
begin;
insert into checkpoint_test values(2, 'after-checkpoint-1');
commit;
--4
-- 第二个检查点
create static_checkpoint;
--5
begin;
insert into checkpoint_test values(3, 'after-checkpoint-2');
-- 不提交

-- 第三个检查点
create static_checkpoint;

begin;
insert into checkpoint_test values(4, 'after-checkpoint-3');
commit;

-- 崩溃
crash

-- 重启后验证
select * from checkpoint_test order by id; -- 应该有id 1,2,4，但没有3

-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------
-----------------------------------------------------------------------------------

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