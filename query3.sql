-- FIXME: this seems not efficient
SELECT COUNT(DISTINCT i.item_id) FROM Item i WHERE (
    (SELECT COUNT(c.category_id) FROM Categorization c WHERE i.item_id = c.item_id ) = 4
);