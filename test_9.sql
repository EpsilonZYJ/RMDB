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