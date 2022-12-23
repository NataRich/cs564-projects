SELECT COUNT(DISTINCT i.seller_id) FROM Item i JOIN User u ON i.seller_id == u.uid AND u.rating > 1000;
