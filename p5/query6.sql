SELECT COUNT(DISTINCT u.uid) FROM User u, Bid b, Item i
    WHERE (u.uid = b.user_id AND u.uid = i.seller_id);