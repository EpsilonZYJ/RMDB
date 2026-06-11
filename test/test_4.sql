CREATE TABLE orders (
    order_id int,
    customer_id int,
    order_date char(40),
    total_amount float
);

CREATE TABLE customers (
    customer_id int,
    name char(50),
    email char(100),
    address char(200)
);

EXPLAIN SELECT * FROM customers c
JOIN orders o ON c.customer_id = o.customer_id
WHERE o.total_amount > 1000;

EXPLAIN SELECT c.name, o.order_id 
FROM customers c 
JOIN orders o ON c.customer_id = o.customer_id;

CREATE TABLE products (
    product_id int,
    name char(50),
    price float
);

CREATE TABLE order_items (
    order_id int,
    product_id int,
    quantity int
);

EXPLAIN SELECT * FROM customers c
JOIN orders o ON c.customer_id = o.customer_id
JOIN order_items oi ON o.order_id = oi.order_id
JOIN products p ON oi.product_id = p.product_id
WHERE o.total_amount > 200;