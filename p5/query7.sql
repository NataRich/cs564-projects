SELECT COUNT(DISTINCT ci.cate_id) FROM Categorization ci, (SELECT DISTINCT i.item_id FROM Item i, Bid b WHERE b.item_id ==i.item_id AND i.currently > 100) z WHERE z.item_id == ci.item_id;
