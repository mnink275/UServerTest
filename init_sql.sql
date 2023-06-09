CREATE TABLE courier_service.couriers(
    courier_id INTEGER PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    courier_type TEXT,
    area INTEGER,
    begin_time TIME,
    end_time TIME
);

INSERT INTO courier_service.couriers(
courier_type, area, begin_time, end_time)
VALUES('Пеший', 1, '08:00', '14:00');

INSERT INTO courier_service.couriers(
courier_type, area, begin_time, end_time)
VALUES('Пеший', 2, '08:00', '16:00');

INSERT INTO courier_service.couriers(
courier_type, area, begin_time, end_time)
VALUES('Велокурьер', 2, '08:00', '16:00');

INSERT INTO courier_service.couriers(
courier_type, area, begin_time, end_time)
VALUES('Автомобиль', 3, '10:00', '17:30');

curl -X POST 'localhost:8080/v1/key-value?courier_id=1&courier_type=Пеший&area=1&begin_time=08:00&end_time=14:00' -i
curl -X GET 'localhost:8080/v1/key-value?courier_id=1' -i

CREATE TABLE courier_service.orders(
    order_id INTEGER PRIMARY KEY GENERATED ALWAYS AS IDENTITY,
    weight REAL,
    area INTEGER,
    delivery_begin_time TIME,
    delivery_end_time TIME
);

INSERT INTO courier_service.orders(
weight, area, delivery_begin_time, delivery_end_time)
VALUES(1.2, 1, '12:00', '14:00');

INSERT INTO courier_service.orders(
weight, area, delivery_begin_time, delivery_end_time)
VALUES(5.0, 2, '09:00', '10:00');

INSERT INTO courier_service.orders(
weight, area, delivery_begin_time, delivery_end_time)
VALUES(3.0, 2, '11:00', '11:30');



CREATE TABLE courier_service.completed_orders(
    courier_id INTEGER REFERENCES couriers(courier_od),
    order_id INTEGER REFERENCES orders(order_id),
    status TEXT
);