#!/bin/bash
# python parser.py items-*.json
sqlite3 ebay.db < create.sql
sqlite3 ebay.db < load.txt