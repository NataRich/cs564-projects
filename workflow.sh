#!/bin/bash
# python parser.py items-*.json
sqlite3 ebayDB < create.sql
sqlite3 ebayDB < load.txt