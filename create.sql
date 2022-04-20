-- check redudency
drop table if exists Category;
drop table if exists User;
drop table if exists Bid;
drop table if exists Item;
drop table if exists Categorization;
drop index if exists idx_categorization_item_id;
drop index if exists idx_categorization_category_id;

-- create tables
create table Category (
    category_id INT PRIMARY KEY NOT NULL,
    name        CHAR(50) UNIQUE NOT NULL
);

create table User (
    uid         CHAR(50) PRIMARY KEY NOT NULL,
    rating      INT NOT NULL,
    location    CHAR(50),
    country     CHAR(50)
);

create table Item (
    item_id     INT PRIMARY KEY NOT NULL,
    seller_id   CHAR(50) NOT NULL,
    name        CHAR(60) NOT NULL,
    currently   DECIMAL NOT NULL,
    buy_price   DECIMAL,
    first_bid   DECIMAL NOT NULL,
    num_bids    INT,
    started     CHAR(50) NOT NULL,
    ends        CHAR(50) NOT NULL,
    desc        TEXT NOT NULL,
    FOREIGN KEY (seller_id) REFERENCES User(uid)
);

create table Categorization (
    categorization_index    INT PRIMARY KEY NOT NULL,
    item_id                 INT NOT NULL,
    category_id             INT NOT NULL,
    FOREIGN KEY (item_id) REFERENCES Item(item_id),
    FOREIGN KEY (category_id) REFERENCES Category(category_id)
);

create table Bid (
    bid_id      INT PRIMARY KEY NOT NULL,
    user_id     CHAR(50) NOT NULL,
    item_id     INT NOT NULL,
    time        CHAR(50) NOT NULL,
    amount      CHAR(50) NOT NULL,
    FOREIGN KEY (user_id) REFERENCES User(uid),
    FOREIGN KEY (item_id) REFERENCES User(item_id)
);

create index idx_categorization_item_id on Categorization (item_id);
create index idx_categorization_category_id on Categorization (category_id);
