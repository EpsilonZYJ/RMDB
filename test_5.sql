-- create table grade (course char(20),id int,score float);
-- insert into grade values('DataStructure',1,95);
-- insert into grade values('DataStructure',2,93.5);
-- insert into grade values('DataStructure',4,87);
-- insert into grade values('DataStructure',3,85);
-- insert into grade values('DB',1,94);
-- insert into grade values('DB',2,74.5);
-- insert into grade values('DB',4,83);
-- insert into grade values('DB',3,87);
-- select MAX(id) as max_id from grade;
-- select MIN(score) as min_score from grade where course = 'DB';
-- select AVG(score) as avg_score from grade where course = 'DataStructure';
-- select COUNT(course) as course_num from grade;
-- select COUNT(*) as row_num from grade;
-- select COUNT(*) as row_num from grade where score < 60;
-- select SUM(score) as sum_score from grade where id = 1;
-- drop table grade;

-- create table grade (course char(20),id int,score float);
-- insert into grade values('DataStructure',1,95);
-- insert into grade values('DataStructure',2,93.5);
-- insert into grade values('DataStructure',3,94.5);
-- insert into grade values('ComputerNetworks',1,99);
-- insert into grade values('ComputerNetworks',2,88.5);
-- insert into grade values('ComputerNetworks',3,92.5);
-- insert into grade values('C++',1,92);
-- insert into grade values('C++',2,89);
-- insert into grade values('C++',3,89.5);
-- select id,MAX(score) as max_score,MIN(score) as min_score,SUM(score) as sum_score from grade group by id;
-- select id,MAX(score) as max_score from grade group by id having COUNT(*) >  3;
-- insert into grade values ('ParallelCompute',1,100);
-- select id,MAX(score) as max_score from grade group by id having COUNT(*) >  3;
-- select id,MAX(score) as max_score,MIN(score) as min_score from grade group by id having COUNT(*) > 1 and MIN(score) > 88;
-- select course ,COUNT(*) as row_num , COUNT(id) as student_num , MAX(score) as top_score, MIN(score) as lowest_score from grade group by course;
-- select course, id, score from grade order by score desc;
-- drop table grade;

-- create table grade (course char(20),id int,score float);
-- insert into grade values('DataStructure',1,95);
-- insert into grade values('DataStructure',2,93.5);
-- insert into grade values('DataStructure',3,94.5);
-- insert into grade values('ComputerNetworks',1,99);
-- insert into grade values('ComputerNetworks',2,88.5);
-- insert into grade values('ComputerNetworks',3,92.5);
-- -- SELECT 列表中不能出现没有在 GROUP BY 子句中的非聚集列
-- select id , score from grade group by course;
-- -- WHERE 子句中不能用聚集函数作为条件表达式
-- select id, MAX(score) as max_score from grade where MAX(score) > 90 group by id;

-- create table grade (course char(20),id int,score float);
-- insert into grade values('DataStructure',1,95);
-- insert into grade values('DataStructure',2,93.5);
-- insert into grade values('DataStructure',4,87);
-- insert into grade values('DataStructure',3,85);
-- insert into grade values('DB',1,94);
-- insert into grade values('DB',2,74.5);
-- insert into grade values('DB',4,83);
-- insert into grade values('DB',3,87);
-- select MAX(id) as max_id from grade;
-- select MIN(score) as min_score from grade where course = 'DB';
-- select COUNT(course) as course_num from grade;
-- select COUNT(*) as row_num from grade;
-- select SUM(score) as sum_score from grade where id = 1;
-- drop table grade;

-- create table grade (course char(20),id int,score float);
-- insert into grade values('DataStructure',1,95);
-- insert into grade values('DataStructure',2,93.5);
-- insert into grade values('DataStructure',3,94.5);
-- insert into grade values('ComputerNetworks',1,99);
-- insert into grade values('ComputerNetworks',2,88.5);
-- insert into grade values('ComputerNetworks',3,92.5);
-- insert into grade values('C++',1,92);
-- insert into grade values('C++',2,89);
-- insert into grade values('C++',3,89.5);
-- select id,MAX(score) as max_score,MIN(score) as min_score,SUM(score) as
-- sum_score from grade group by id;
-- select id,MAX(score) as max_score from grade group by id having COUNT(*) >3;
-- insert into grade values ('ParallelCompute',1,100);
-- select id,MAX(score) as max_score from grade group by id having COUNT(*) >3;
-- select id,MAX(score) as max_score,MIN(score) as min_score from grade group
-- by id having COUNT(*) > 1 and MIN(score) > 88;
-- select course ,COUNT(*) as row_num , COUNT(id) as student_num , MAX(score)
-- as top_score, MIN(score) as lowest_score from grade group by course;
-- drop table grade;

-- create table grade (course char(20),id int,score float);
-- insert into grade values('DataStructure',1,95);
-- insert into grade values('DataStructure',2,93.5);
-- insert into grade values('DataStructure',3,94.5);
-- insert into grade values('ComputerNetworks',1,99);
-- insert into grade values('ComputerNetworks',2,88.5);
-- insert into grade values('ComputerNetworks',3,92.5);
-- -- SELECT 列表中不能出现没有在 GROUP BY ⼦句中的⾮聚集列
-- select id , score from grade group by course;
-- -- WHERE ⼦句中不能⽤聚集函数作为条件表达式
-- select id, MAX(score) as max_score where MAX(score) > 90 from grade group
-- by id;

create table records (vendor char(5), invoice_number int, amount float);
insert into records values('alpha', 1001, 98.0);
insert into records values('bravo', 2002, 76.5);
insert into records values('charl', 3003, 99.0);
insert into records values('delta', 1001, 98.5);
insert into records values('echoo', 4004, 88.25);
insert into records values('foxxx', 4004, 77.0);
insert into records values('golfy', 5005, 97.75);
insert into records values('hotel', 5005, 86.75);
insert into records values('indio', 6006, 76.25);
insert into records values('julie', 3003, 88.0);
insert into records values('karen', 5005, 89.25);
insert into records values('lenny', 2002, 91.125);
insert into records values('mango', 6006, 98.5);
insert into records values('nancy', 1001, 89.75);
insert into records values('oscar', 2002, 90.0);
insert into records values('peter', 3003, 95.0);
insert into records values('quack', 6006, 88.625);
insert into records values('romeo', 4004, 92.0);
insert into records values('sunny', 1001, 95.25);
insert into records values('tonny', 7007, 98.125);
insert into records values('ultra', 4004, 91.5);
insert into records values('vivid', 7007, 98.3125);
select * from records order by invoice_number asc limit 2;