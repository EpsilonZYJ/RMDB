-- ====================================================================
-- 测试点1: 尝试建表 - 验证基本数据类型的表创建和删除功能
-- ====================================================================

-- 测试基本数据类型
CREATE TABLE data_types_test (
    id INT,
    num_value INT,
    decimal_num FLOAT,
    name CHAR(50),
    description CHAR(200)
);

-- 验证表是否创建成功
SELECT * FROM data_types_test;

-- 测试创建包含多种数据类型的表
CREATE TABLE students (
    student_id INT,
    student_name CHAR(50),
    age INT,
    gpa FLOAT,
    major CHAR(30),
    enrollment_date CHAR(10)
);

-- 测试创建第二个表(用于后续的连接查询测试)
CREATE TABLE courses (
    course_id INT,
    course_name CHAR(100),
    credits INT,
    department CHAR(50),
    instructor CHAR(50)
);

-- 测试创建第三个表(用于后续的连接查询测试)
CREATE TABLE enrollments (
    enrollment_id INT,
    student_id INT,
    course_id INT,
    grade FLOAT,
    semester CHAR(20)
);

-- 测试创建表格用于后续更新和删除操作
CREATE TABLE employees (
    emp_id INT,
    first_name CHAR(30),
    last_name CHAR(30),
    department CHAR(30),
    position CHAR(30),
    salary FLOAT,
    hire_date CHAR(10)
);

-- 测试创建包含多列的表
CREATE TABLE products (
    product_id INT,
    product_name CHAR(100),
    category CHAR(30),
    subcategory CHAR(30),
    price FLOAT,
    cost FLOAT,
    stock_quantity INT,
    supplier_id INT,
    restock_date CHAR(10)
);

-- 测试DROP TABLE功能
CREATE TABLE temp_table (id INT, value CHAR(10));
DROP TABLE temp_table;

-- 验证表是否已被删除
-- 预期应该显示错误，因为表已不存在
SELECT * FROM temp_table;

-- 测试重复创建表
CREATE TABLE duplicate_test (id INT);
-- 尝试再次创建相同的表(预期失败)
CREATE TABLE duplicate_test (id INT);

-- 测试完成后清理该测试表
DROP TABLE duplicate_test;

-- ====================================================================
-- 测试点2: 单表插入与条件查询
-- ====================================================================

-- 向students表插入数据
INSERT INTO students VALUES (1, 'Alice Johnson', 20, 3.75, 'Computer Science', '2022-09-01');
INSERT INTO students VALUES (2, 'Bob Smith', 22, 3.45, 'Mathematics', '2021-09-01');
INSERT INTO students VALUES (3, 'Charlie Brown', 19, 3.91, 'Physics', '2023-09-01');
INSERT INTO students VALUES (4, 'Diana Miller', 21, 3.62, 'Computer Science', '2022-01-15');
INSERT INTO students VALUES (5, 'Edward Wilson', 23, 3.20, 'Business', '2020-09-01');
INSERT INTO students VALUES (6, 'Fiona Garcia', 20, 3.88, 'Mathematics', '2022-09-01');
INSERT INTO students VALUES (7, 'George Martinez', 21, 3.50, 'Computer Science', '2022-01-15');
INSERT INTO students VALUES (8, 'Hannah Lee', 22, 3.30, 'Biology', '2021-09-01');
INSERT INTO students VALUES (9, 'Ian Clark', 19, 3.95, 'Physics', '2023-09-01');
INSERT INTO students VALUES (10, 'Julia Wright', 23, 3.10, 'Business', '2020-09-01');

-- 向courses表插入数据
INSERT INTO courses VALUES (101, 'Introduction to Programming', 3, 'Computer Science', 'Prof. Anderson');
INSERT INTO courses VALUES (102, 'Data Structures', 4, 'Computer Science', 'Prof. Brown');
INSERT INTO courses VALUES (103, 'Calculus I', 4, 'Mathematics', 'Prof. Chen');
INSERT INTO courses VALUES (104, 'Calculus II', 4, 'Mathematics', 'Prof. Davis');
INSERT INTO courses VALUES (105, 'Classical Physics', 3, 'Physics', 'Prof. Einstein');
INSERT INTO courses VALUES (106, 'Quantum Mechanics', 4, 'Physics', 'Prof. Feynman');
INSERT INTO courses VALUES (107, 'Business Ethics', 3, 'Business', 'Prof. Gates');
INSERT INTO courses VALUES (108, 'Marketing 101', 3, 'Business', 'Prof. Hill');
INSERT INTO courses VALUES (109, 'Molecular Biology', 4, 'Biology', 'Prof. Ivy');
INSERT INTO courses VALUES (110, 'Genetics', 3, 'Biology', 'Prof. Jones');

-- 向enrollments表插入数据
INSERT INTO enrollments VALUES (1001, 1, 101, 95.0, 'Fall 2022');
INSERT INTO enrollments VALUES (1002, 1, 103, 88.0, 'Fall 2022');
INSERT INTO enrollments VALUES (1003, 2, 103, 91.0, 'Fall 2021');
INSERT INTO enrollments VALUES (1004, 2, 104, 87.0, 'Spring 2022');
INSERT INTO enrollments VALUES (1005, 3, 105, 98.0, 'Fall 2023');
INSERT INTO enrollments VALUES (1006, 3, 106, 94.0, 'Fall 2023');
INSERT INTO enrollments VALUES (1007, 4, 101, 90.0, 'Spring 2022');
INSERT INTO enrollments VALUES (1008, 4, 102, 92.0, 'Fall 2022');
INSERT INTO enrollments VALUES (1009, 5, 107, 85.0, 'Fall 2020');
INSERT INTO enrollments VALUES (1010, 5, 108, 88.0, 'Spring 2021');

-- 向employees表插入数据
INSERT INTO employees VALUES (101, 'John', 'Doe', 'Engineering', 'Software Engineer', 85000.0, '2020-01-15');
INSERT INTO employees VALUES (102, 'Jane', 'Smith', 'Engineering', 'Senior Developer', 95000.0, '2019-05-20');
INSERT INTO employees VALUES (103, 'Michael', 'Johnson', 'Marketing', 'Marketing Specialist', 75000.0, '2021-03-10');
INSERT INTO employees VALUES (104, 'Emily', 'Davis', 'HR', 'HR Manager', 90000.0, '2018-11-01');
INSERT INTO employees VALUES (105, 'David', 'Wilson', 'Engineering', 'QA Engineer', 80000.0, '2020-07-15');

-- 向products表插入数据
INSERT INTO products VALUES (1001, 'Laptop Pro', 'Electronics', 'Computers', 1299.99, 900.00, 50, 501, '2023-05-15');
INSERT INTO products VALUES (1002, 'Smartphone X', 'Electronics', 'Phones', 899.99, 600.00, 75, 502, '2023-06-01');
INSERT INTO products VALUES (1003, 'Office Chair', 'Furniture', 'Office', 199.99, 120.00, 30, 503, '2023-04-10');
INSERT INTO products VALUES (1004, 'Coffee Table', 'Furniture', 'Living Room', 299.99, 180.00, 15, 503, '2023-03-20');
INSERT INTO products VALUES (1005, 'Bluetooth Headphones', 'Electronics', 'Audio', 149.99, 80.00, 100, 501, '2023-06-15');

-- 测试不同形式的INSERT语句
-- 指定列插入
INSERT INTO students (student_id, student_name, age, major) VALUES (11, 'Kevin Thompson', 20, 'Chemistry');

-- 一次插入多行
INSERT INTO courses VALUES (111, 'Organic Chemistry', 4, 'Chemistry', 'Prof. Keller'),
INSERT INTO courses VALUES (112, 'Inorganic Chemistry', 3, 'Chemistry', 'Prof. Lawrence');

-- 基本条件查询测试
-- 相等条件查询
SELECT * FROM students WHERE major = 'Computer Science';

-- 比较条件查询
SELECT * FROM students WHERE age > 21;

-- AND条件查询
SELECT * FROM students WHERE major = 'Physics' AND gpa > 3.9;

-- OR条件查询
SELECT * FROM students WHERE major = 'Mathematics' OR major = 'Business';

-- 简单的大于小于条件
SELECT * FROM students WHERE gpa > 3.5;
SELECT * FROM students WHERE gpa < 3.5;

-- 多条件复杂查询
SELECT * FROM employees WHERE department = 'Engineering' AND salary > 85000.0;

-- 按列名选择查询
SELECT student_id, student_name, major FROM students WHERE gpa > 3.8;

-- 查询所有专业为Computer Science的学生姓名和GPA
SELECT student_name, gpa FROM students WHERE major = 'Computer Science';

-- ====================================================================
-- 测试点3: 单表更新与条件查询
-- ====================================================================

-- 基本更新操作
UPDATE employees SET salary = 88000.0 WHERE emp_id = 101;

-- 验证更新是否成功
SELECT * FROM employees WHERE emp_id = 101;

-- 条件更新
UPDATE employees SET salary = salary * 1.1 WHERE department = 'Engineering';

-- 验证条件更新
SELECT * FROM employees WHERE department = 'Engineering';

-- 多字段更新
UPDATE employees SET position = 'Senior Software Engineer', salary = 92000.0 WHERE emp_id = 101;

-- 验证多字段更新
SELECT * FROM employees WHERE emp_id = 101;

-- 基于其他字段的计算更新
UPDATE employees SET salary = salary + 5000.0 WHERE position = 'Senior Developer';

-- 验证基于计算的更新
SELECT * FROM employees WHERE position = 'Senior Developer';

-- 更新产品价格和库存
UPDATE products SET price = 1199.99, stock_quantity = 45 WHERE product_id = 1001;

-- 验证产品更新
SELECT * FROM products WHERE product_id = 1001;

-- 根据条件批量更新
UPDATE products SET price = price * 0.9 WHERE category = 'Electronics';

-- 验证批量更新
SELECT * FROM products WHERE category = 'Electronics';

-- 测试更新后的条件查询
-- 查找所有工资超过90000的员工
SELECT * FROM employees WHERE salary > 90000.0;

-- 查找特定部门的员工
SELECT * FROM employees WHERE department = 'Engineering';

-- ====================================================================
-- 测试点4: 单表删除与条件查询
-- ====================================================================

-- 删除特定记录
DELETE FROM employees WHERE emp_id = 105;

-- 验证删除是否成功
SELECT * FROM employees WHERE emp_id = 105;

-- 条件删除
DELETE FROM employees WHERE department = 'Finance';

-- 验证条件删除
SELECT * FROM employees WHERE department = 'Finance';

-- 删除产品库存中的特定商品
DELETE FROM products WHERE stock_quantity < 20;

-- 验证产品删除
SELECT * FROM products WHERE stock_quantity < 20;

-- 查询剩余的产品
SELECT * FROM products;

-- 删除后的条件查询
-- 查询特定部门的员工
SELECT * FROM employees WHERE department = 'Engineering';

-- 查询特定职位的员工
SELECT * FROM employees WHERE position = 'Senior Developer';

-- ====================================================================
-- 测试点5: 连接查询
-- ====================================================================

-- 基本内连接 - 查询学生及其所选课程
SELECT s.student_id, s.student_name, c.course_name
FROM students s, courses c, enrollments e
WHERE s.student_id = e.student_id AND c.course_id = e.course_id;

-- 带条件的连接查询 - 查找计算机科学专业的学生及其所选课程
SELECT s.student_id, s.student_name, c.course_name
FROM students s, courses c, enrollments e
WHERE s.student_id = e.student_id AND c.course_id = e.course_id AND s.major = 'Computer Science';

-- 带成绩条件的连接查询 - 查找成绩大于90的学生和课程
SELECT s.student_id, s.student_name, c.course_name, e.grade
FROM students s, courses c, enrollments e
WHERE s.student_id = e.student_id AND c.course_id = e.course_id AND e.grade > 90.0;

-- 多表连接查询 - 查找物理学专业的学生，他们选修的物理系开设的课程及成绩
SELECT s.student_id, s.student_name, c.course_name, e.grade
FROM students s, courses c, enrollments e
WHERE s.student_id = e.student_id AND c.course_id = e.course_id 
  AND s.major = 'Physics' AND c.department = 'Physics';

-- 复杂连接查询 - 查找GPA大于3.5，同时选修了特定学分课程的学生
SELECT s.student_id, s.student_name, s.gpa, c.course_name, c.credits
FROM students s, courses c, enrollments e
WHERE s.student_id = e.student_id AND c.course_id = e.course_id
  AND s.gpa > 3.5 AND c.credits >= 4;

-- 使用别名简化的连接查询
SELECT s.student_name, c.course_name, e.grade
FROM students s, courses c, enrollments e
WHERE s.student_id = e.student_id AND c.course_id = e.course_id AND e.grade > 95.0;

-- 综合测试 - 查找计算机科学和数学专业学生选修的计算机科学系课程
SELECT s.student_id, s.student_name, s.major, c.course_name
FROM students s, courses c, enrollments e
WHERE s.student_id = e.student_id AND c.course_id = e.course_id
  AND (s.major = 'Computer Science' OR s.major = 'Mathematics')
  AND c.department = 'Computer Science';