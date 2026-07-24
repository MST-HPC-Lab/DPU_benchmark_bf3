#!/bin/bash

# nohup ./search_suite.sh &

echo "RUNNING spacev SEARCHES (flat)"
python3 index_searcher.py --only "flat"    --indexes_dir "spacev"
echo "RUNNING spacev SEARCHES (lsh)"
python3 index_searcher.py --only "lsh"     --indexes_dir "spacev"
echo "RUNNING spacev SEARCHES (pq)"
python3 index_searcher.py --only "pq"      --indexes_dir "spacev"
echo "RUNNING spacev SEARCHES (ivfpq)"
python3 index_searcher.py --only "ivfpq"   --indexes_dir "spacev"
echo "RUNNING spacev SEARCHES (hnsw)"
python3 index_searcher.py --only "hnsw"    --indexes_dir "spacev"
echo "RUNNING spacev SEARCHES (hnsw_pq)"
python3 index_searcher.py --only "hnsw_pq" --indexes_dir "spacev"
echo "RUNNING spacev SEARCHES (hnsw_sq)"
python3 index_searcher.py --only "hnsw_sq" --indexes_dir "spacev"