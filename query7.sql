-- NOTE: bug exists in this query when conducting string comparison; considering 
-- modiying the type of i.currently into REAL
-- another issue: does "with a bid of more than $100" refer to buy_price?
WITH b AS (
    SELECT i.item_id AS item_id FROM Item i WHERE i.buy_price > 1000
)
SELECT COUNT(DISTINCT c.category_id) FROM Categorization c, b WHERE (c.item_id = b.item_id);