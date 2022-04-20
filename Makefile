# This file serve as a tester and SHOULD NOT be submitted.
DB_NAME = ebay.db

create:
	sqlite3 $(DB_NAME) < create.sql
	sqlite3 $(DB_NAME) < load.txt

test: 
# 3 is not efficient; we omit it here.
	@for i in 1 2 3 4 5 6 7; do \
		sqlite3 $(DB_NAME) < query$$i.sql; \
	done

