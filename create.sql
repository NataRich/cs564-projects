-- check redudency
drop table if exists Category;
drop table if exists User;
drop table if exists Bid;
drop table if exists Item;
drop table if exists Categorization;

-- create tables
create table Category (
    category_id INT PRIMARY KEY NOT NULL,
    name        CHAR(50) NOT NULL
);

create table Categorization (
    category_index       INT PRIMARY KEY NOT NULL,
    itemId      INT NOT NULL,
    counting    INT NOT NULL
);

create table Bid (
    bid_id      INT PRIMARY KEY NOT NULL,
    user_id     CHAR(50) NOT NULL,
    item_id     INT NOT NULL,
    time        CHAR(50) NOT NULL,
    amount      CHAR(50) NOT NULL
);

create table Item (
    item_id     INT PRIMARY KEY NOT NULL,
    seller_id   CHAR(50) NOT NULL,
    name        CHAR(60) NOT NULL,
    currently   CHAR(50) NOT NULL,
    buy_price   CHAR(50),
    first_bid   CHAR(50) NOT NULL,
    num_bids    INT,
    started     CHAR(50) NOT NULL,
    ends        CHAR(50) NOT NULL,
    desc        TEXT NOT NULL
);

create table User (
    uid         CHAR(50) PRIMARY KEY NOT NULL,
    rating      INT NOT NULL,
    location    CHAR(50),
    country     CHAR(50)
);
