#!/bin/bash

# nohup ./search_suite.sh &

# echo "RUNNING spacev SEARCHES (flat)"
# python3 index_searcher.py --only "flat" --k 10   --indexes_dir "spacev" 
echo "RUNNING spacev SEARCHES (lsh)"
python3 index_searcher.py --only "lsh" --k 10    --indexes_dir "spacev"
echo "RUNNING spacev SEARCHES (pq)"
python3 index_searcher.py --only "pq" --k 10     --indexes_dir "spacev"
echo "RUNNING spacev SEARCHES (ivfpq)"
python3 index_searcher.py --only "ivfpq" --k 10  --indexes_dir "spacev"
# echo "RUNNING spacev SEARCHES (hnsw)"
# python3 index_searcher.py --only "hnsw" --k 10   --indexes_dir "spacev"
# echo "RUNNING spacev SEARCHES (hnsw_pq)"
# python3 index_searcher.py --only "hnsw_pq" --k 10 --indexes_dir "spacev"
echo "RUNNING spacev SEARCHES (hnsw_sq)"
python3 index_searcher.py --only "hnsw_sq" --k 10 --indexes_dir "spacev"

