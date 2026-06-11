-- 创建多表和索引
create table a (id int, v char(10));
create table b (id int, v char(10));
create table c (id int, v char(10));
create index idx_a_id on a(id);
create index idx_b_id on b(id);

-- 1. 基本插入+提交+崩溃
begin;
insert into a values(1, 'a1');
insert into a values(2, 'a2');
commit;
crash
select * from a order by id; -- 应有1,2

-- 2. 插入+未提交+崩溃
begin;
insert into a values(3, 'a3');
crash
select * from a order by id; -- 3不应出现

-- 3. 插入+提交+检查点+崩溃
begin;
insert into a values(4, 'a4');
commit;
create static_checkpoint;
crash
select * from a order by id; -- 应有1,2,4

-- 4. 插入+未提交+检查点+崩溃
begin;
insert into a values(5, 'a5');
create static_checkpoint;
crash
select * from a order by id; -- 5不应出现

-- 5. 插入+提交+检查点+更新+提交+崩溃
begin;
insert into a values(6, 'a6');
commit;
create static_checkpoint;
begin;
update a set v = 'a6x' where id = 6;
commit;
crash
select * from a where id = 6; -- v应为a6x

-- 6. 插入+提交+检查点+删除+未提交+崩溃
begin;
insert into a values(7, 'a7');
commit;
create static_checkpoint;
begin;
delete from a where id = 7;
-- 未提交
crash
select * from a where id = 7; -- 7应还在

-- 7. 插入+提交+检查点+删除+提交+崩溃
begin;
insert into a values(8, 'a8');
commit;
create static_checkpoint;
begin;
delete from a where id = 8;
commit;
crash
select * from a where id = 8; -- 8不应出现

-- 8. 多事务并发+部分提交+崩溃
begin; -- t1
insert into b values(1, 'b1');
begin; -- t2
insert into b values(2, 'b2');
commit; -- t2
-- t1未提交
crash
select * from b order by id; -- 只应有2

-- 9. DDL+插入+崩溃
create table d (id int, v char(10));
begin;
insert into d values(1, 'd1');
commit;
crash
select * from d; -- 应有1

-- 10. 索引插入+更新+崩溃
begin;
insert into a values(9, 'a9');
commit;
create static_checkpoint;
begin;
update a set id = 10 where id = 9;
commit;
crash
select * from a where id = 10; -- 应有一条
select * from a where id = 9;  -- 不应有

-- 11. 回滚测试
begin;
insert into a values(11, 'a11');
rollback;
crash
select * from a where id = 11; -- 不应有

-- 12. 检查点后未提交多操作
begin;
insert into a values(12, 'a12');
insert into a values(13, 'a13');
create static_checkpoint;
insert into a values(14, 'a14');
-- 未提交
crash
select * from a where id >= 12; -- 12,13,14都不应有

-- 13. 多表操作+崩溃
begin;
insert into a values(15, 'a15');
insert into b values(3, 'b3');
commit;
crash
select * from a where id = 15; -- 应有
select * from b where id = 3;  -- 应有

-- 14. 检查点后DDL
create static_checkpoint;
create table e (id int, v char(10));
begin;
insert into e values(1, 'e1');
commit;
crash
select * from e; -- 应有1

-- 15. 检查点后未提交DDL
create static_checkpoint;
create table f (id int, v char(10));
-- 未提交插入
begin;
insert into f values(1, 'f1');
-- 未提交
crash
select * from f; -- 应为空或表不存在

-- 16. 复杂事务混合
begin;
insert into a values(16, 'a16');
insert into b values(4, 'b4');
commit;
begin;
insert into a values(17, 'a17');
-- 未提交
crash
select * from a where id = 16; -- 应有
select * from a where id = 17; -- 不应有
select * from b where id = 4;  -- 应有

-- 17. 跨表回滚
begin;
insert into a values(18, 'a18');
insert into b values(5, 'b5');
rollback;
crash
select * from a where id = 18; -- 不应有
select * from b where id = 5;  -- 不应有

-- 18. 并发交错提交
begin; -- t1
insert into c values(1, 'c1');
begin; -- t2
insert into c values(2, 'c2');
commit; -- t2
insert into c values(3, 'c3');
commit; -- t1
crash
select * from c order by id; -- 应有1,2,3

-- 19. 多表多事务交错
begin; -- t1
insert into a values(19, 'a19');
begin; -- t2
insert into b values(6, 'b6');
commit; -- t2
insert into c values(4, 'c4');
commit; -- t1
crash
select * from a where id = 19; -- 应有
select * from b where id = 6;  -- 应有
select * from c where id = 4;  -- 应有

-- 20. 边界case：空表、无操作、DDL后无DML
create table g (id int, v char(10));
crash
select * from g; -- 应为空

-- 21. 边界case：多次崩溃恢复
begin;
insert into a values(20, 'a20');
commit;
crash
crash
crash
select * from a where id = 20; -- 应有

-- 22. 边界case：DDL后立即崩溃
create table h (id int, v char(10));
crash
select * from h; -- 应为空

-- 23. 边界case：DDL+未提交DML+崩溃
create table i (id int, v char(10));
begin;
insert into i values(1, 'i1');
crash
select * from i; -- 应为空

-- 24. 边界case：DDL+提交DML+崩溃
create table j (id int, v char(10));
begin;
insert into j values(1, 'j1');
commit;
crash
select * from j; -- 应有1

-- 25. 多表多事务多DDL混合
create table k (id int, v char(10));
begin;
insert into k values(1, 'k1');
commit;
create static_checkpoint;
begin;
insert into k values(2, 'k2');
insert into a values(21, 'a21');
commit;
crash
select * from k; -- 应有1,2
select * from a where id = 21; -- 应有

-- 26. 多次检查点+未提交
create static_checkpoint;
begin;
insert into a values(22, 'a22');
create static_checkpoint;
insert into a values(23, 'a23');
-- 未提交
crash
select * from a where id = 22; -- 不应有
select * from a where id = 23; -- 不应有

-- 27. 多表多索引混合
create table l (id int, v char(10));
create index idx_l_id on l(id);
begin;
insert into l values(1, 'l1');
insert into l values(2, 'l2');
commit;
create static_checkpoint;
begin;
update l set id = 3 where id = 2;
commit;
crash
select * from l order by id; -- 应有1,3
select * from l where id = 2; -- 不应有
select * from l where id = 3; -- 应有

-- 28. 大事务+崩溃
begin;
insert into a values(100, 'big');
insert into a values(101, 'big');
insert into a values(102, 'big');
insert into a values(103, 'big');
insert into a values(104, 'big');
commit;
crash
select * from a where id >= 100; -- 应有100~104

-- 29. 事务内DDL
begin;
create table m (id int, v char(10));
insert into m values(1, 'm1');
commit;
crash
select * from m; -- 应有1

-- 30. 事务内DDL+回滚
begin;
create table n (id int, v char(10));
insert into n values(1, 'n1');
rollback;
crash
select * from n; -- 应为空或表不存在