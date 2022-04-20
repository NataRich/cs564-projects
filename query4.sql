-- NOTE: bug exists in this query when conducting string comparison; considering 
-- modiying the type of i.currently into REAL
SELECT (i.item_id) FROM Item i WHERE (
    i.currently = (SELECT MAX(i.currently) FROM Item i)
);