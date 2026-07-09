#!/bin/bash

# nohup ./search_suite.sh &

echo "RUNNING glove SEARCHES"
python3 index_searcher.py --only "flat"    --indexes_dir "glove"
python3 index_searcher.py --only "lsh"     --indexes_dir "glove"
python3 index_searcher.py --only "pq"      --indexes_dir "glove"
python3 index_searcher.py --only "ivfpq"   --indexes_dir "glove"
python3 index_searcher.py --only "hnsw"    --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_pq" --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_sq" --indexes_dir "glove"

echo "RUNNING glove SEARCHES WITH 16 THREADS"
python3 index_searcher.py --only "flat"    --threads "16" --indexes_dir "glove"
python3 index_searcher.py --only "lsh"     --threads "16" --indexes_dir "glove"
python3 index_searcher.py --only "pq"      --threads "16" --indexes_dir "glove"
python3 index_searcher.py --only "ivfpq"   --threads "16" --indexes_dir "glove"
python3 index_searcher.py --only "hnsw"    --threads "16" --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --indexes_dir "glove"

echo "RUNNING glove SEARCHES WITH VARIED THREADS AND FIXED k=10"
python3 index_searcher.py --only "flat"    --threads "12" --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "lsh"     --threads "12" --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "pq"      --threads "12" --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "ivfpq"   --threads "12" --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw"    --threads "12" --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_pq" --threads "12" --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_sq" --threads "12" --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "flat"    --threads "8"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "lsh"     --threads "8"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "pq"      --threads "8"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "ivfpq"   --threads "8"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw"    --threads "8"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_pq" --threads "8"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_sq" --threads "8"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "flat"    --threads "4"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "lsh"     --threads "4"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "pq"      --threads "4"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "ivfpq"   --threads "4"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw"    --threads "4"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_pq" --threads "4"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_sq" --threads "4"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "flat"    --threads "2"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "lsh"     --threads "2"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "pq"      --threads "2"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "ivfpq"   --threads "2"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw"    --threads "2"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_pq" --threads "2"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_sq" --threads "2"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "flat"    --threads "1"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "lsh"     --threads "1"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "pq"      --threads "1"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "ivfpq"   --threads "1"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw"    --threads "1"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_pq" --threads "1"  --k 10 --indexes_dir "glove"
python3 index_searcher.py --only "hnsw_sq" --threads "1"  --k 10 --indexes_dir "glove"

echo "RUNNING fasttext SEARCHES"
python3 index_searcher.py --only "flat"    --indexes_dir "fasttext"
python3 index_searcher.py --only "lsh"     --indexes_dir "fasttext"
python3 index_searcher.py --only "pq"      --indexes_dir "fasttext"
python3 index_searcher.py --only "ivfpq"   --indexes_dir "fasttext"
python3 index_searcher.py --only "hnsw"    --indexes_dir "fasttext"
python3 index_searcher.py --only "hnsw_pq" --indexes_dir "fasttext"
python3 index_searcher.py --only "hnsw_sq" --indexes_dir "fasttext"

echo "RUNNING fasttext SEARCHES WITH 16 THREADS"
python3 index_searcher.py --only "flat"    --threads "16" --indexes_dir "fasttext"
python3 index_searcher.py --only "lsh"     --threads "16" --indexes_dir "fasttext"
python3 index_searcher.py --only "pq"      --threads "16" --indexes_dir "fasttext"
python3 index_searcher.py --only "ivfpq"   --threads "16" --indexes_dir "fasttext"
python3 index_searcher.py --only "hnsw"    --threads "16" --indexes_dir "fasttext"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --indexes_dir "fasttext"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --indexes_dir "fasttext"

echo "RUNNING sift SEARCHES"
python3 index_searcher.py --only "flat"    --indexes_dir "sift"
python3 index_searcher.py --only "lsh"     --indexes_dir "sift"
python3 index_searcher.py --only "pq"      --indexes_dir "sift"
python3 index_searcher.py --only "ivfpq"   --indexes_dir "sift"
python3 index_searcher.py --only "hnsw"    --indexes_dir "sift"
python3 index_searcher.py --only "hnsw_pq" --indexes_dir "sift"
python3 index_searcher.py --only "hnsw_sq" --indexes_dir "sift"

echo "RUNNING sift SEARCHES WITH 16 THREADS"
python3 index_searcher.py --only "flat"    --threads "16" --indexes_dir "sift"
python3 index_searcher.py --only "lsh"     --threads "16" --indexes_dir "sift"
python3 index_searcher.py --only "pq"      --threads "16" --indexes_dir "sift"
python3 index_searcher.py --only "ivfpq"   --threads "16" --indexes_dir "sift"
python3 index_searcher.py --only "hnsw"    --threads "16" --indexes_dir "sift"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --indexes_dir "sift"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --indexes_dir "sift"

echo "RUNNING EQUALIZED DATASET SEARCHES WITH FIXED k=10"
python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "sift_128d_400k"

echo "RUNNING EQUALIZED DATASET SEARCHES WITH 16 THREADS AND FIXED k=10"
python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "sift_128d_400k"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "sift_128d_400k"

echo "RUNNING sift SEARCHES WITH SCALING DATASET SIZE AND FIXED k=10"
python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "sift_10m"

echo "RUNNING sift SEARCHES WITH SCALING DATASET SIZE AND 16 THREADS WITH FIXED k=10"
python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "sift_100k"
python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "sift_200k"
python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "sift_500k"
python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "sift_1m"
python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "sift_2m"
python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "sift_5m"
python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "sift_10m"
python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "sift_10m"

# echo "RUNNING 10m PART OF LAST TWO SEARCHES"
# python3 index_searcher.py --only "flat"    --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "lsh"     --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "pq"      --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "ivfpq"   --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "hnsw"    --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "hnsw_pq" --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "hnsw_sq" --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "flat"    --threads "16" --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "lsh"     --threads "16" --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "pq"      --threads "16" --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "ivfpq"   --threads "16" --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "hnsw"    --threads "16" --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "hnsw_pq" --threads "16" --k "10" --indexes_dir "sift_10m"
# python3 index_searcher.py --only "hnsw_sq" --threads "16" --k "10" --indexes_dir "sift_10m"



# if grep -q "BlueField" /sys/class/dmi/id/product_name; then
#     echo "DETECTED BLUEFIELD ENVIRONMENT"
    
#     # BF3 BUILD
#     # python3 index_builder.py --file glove.6B.200d.txt --out_folder glove >> results/glove/bf3_build.txt 2>>&1 & disown
#     # python3 index_builder.py --file fasttext_cc.en.300.vec --out_folder fasttext >> results/fasttext/bf3_build.txt 2>>&1 & disown

#     # BF3 SEARCH
#     #### Glove
#     python3 index_searcher.py --only "flat"  --threads 16 --indexes_dir glove >> results/glove/bf3_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 16 --indexes_dir glove >> results/glove/bf3_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 16 --indexes_dir glove >> results/glove/bf3_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 16 --indexes_dir glove >> results/glove/bf3_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 16 --indexes_dir glove >> results/glove/bf3_hnsw.txt  2>>&1

#     python3 index_searcher.py --only "flat"  --threads 8  --indexes_dir glove >> results/glove/bf3_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 8  --indexes_dir glove >> results/glove/bf3_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 8  --indexes_dir glove >> results/glove/bf3_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 8  --indexes_dir glove >> results/glove/bf3_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 8  --indexes_dir glove >> results/glove/bf3_hnsw.txt  2>>&1
    
#     python3 index_searcher.py --only "flat"  --threads 4  --indexes_dir glove >> results/glove/bf3_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 4  --indexes_dir glove >> results/glove/bf3_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 4  --indexes_dir glove >> results/glove/bf3_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 4  --indexes_dir glove >> results/glove/bf3_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 4  --indexes_dir glove >> results/glove/bf3_hnsw.txt  2>>&1
    
#     python3 index_searcher.py --only "flat"  --threads 2  --indexes_dir glove >> results/glove/bf3_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 2  --indexes_dir glove >> results/glove/bf3_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 2  --indexes_dir glove >> results/glove/bf3_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 2  --indexes_dir glove >> results/glove/bf3_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 2  --indexes_dir glove >> results/glove/bf3_hnsw.txt  2>>&1
    
#     python3 index_searcher.py --only "flat"  --threads 1  --indexes_dir glove >> results/glove/bf3_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 1  --indexes_dir glove >> results/glove/bf3_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 1  --indexes_dir glove >> results/glove/bf3_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 1  --indexes_dir glove >> results/glove/bf3_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 1  --indexes_dir glove >> results/glove/bf3_hnsw.txt  2>>&1

#     #### Fasttext
#     python3 index_searcher.py --only "flat"  --threads 16 --indexes_dir fasttext >> results/fasttext/bf3_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 16 --indexes_dir fasttext >> results/fasttext/bf3_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 16 --indexes_dir fasttext >> results/fasttext/bf3_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 16 --indexes_dir fasttext >> results/fasttext/bf3_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 16 --indexes_dir fasttext >> results/fasttext/bf3_hnsw.txt  2>>&1

#     python3 index_searcher.py --only "flat"  --threads 8  --indexes_dir fasttext >> results/fasttext/bf3_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 8  --indexes_dir fasttext >> results/fasttext/bf3_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 8  --indexes_dir fasttext >> results/fasttext/bf3_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 8  --indexes_dir fasttext >> results/fasttext/bf3_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 8  --indexes_dir fasttext >> results/fasttext/bf3_hnsw.txt  2>>&1
    
#     python3 index_searcher.py --only "flat"  --threads 4  --indexes_dir fasttext >> results/fasttext/bf3_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 4  --indexes_dir fasttext >> results/fasttext/bf3_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 4  --indexes_dir fasttext >> results/fasttext/bf3_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 4  --indexes_dir fasttext >> results/fasttext/bf3_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 4  --indexes_dir fasttext >> results/fasttext/bf3_hnsw.txt  2>>&1
    
#     python3 index_searcher.py --only "flat"  --threads 2  --indexes_dir fasttext >> results/fasttext/bf3_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 2  --indexes_dir fasttext >> results/fasttext/bf3_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 2  --indexes_dir fasttext >> results/fasttext/bf3_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 2  --indexes_dir fasttext >> results/fasttext/bf3_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 2  --indexes_dir fasttext >> results/fasttext/bf3_hnsw.txt  2>>&1
    
#     python3 index_searcher.py --only "flat"  --threads 1  --indexes_dir fasttext >> results/fasttext/bf3_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 1  --indexes_dir fasttext >> results/fasttext/bf3_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 1  --indexes_dir fasttext >> results/fasttext/bf3_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 1  --indexes_dir fasttext >> results/fasttext/bf3_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 1  --indexes_dir fasttext >> results/fasttext/bf3_hnsw.txt  2>>&1

# else
#     echo "DETECTED HOST ENVIRONMENT"
    
#     # Host BUILD
#     # python3 index_builder.py --file glove.6B.200d.txt --out_folder glove >> results/glove/bf3host_build.txt 2>>&1 & disown
#     # python3 index_builder.py --file fasttext_cc.en.300.vec --out_folder fasttext >> results/fasttext/bf3host_build.txt 2>>&1 & disown

#     # Host SEARCH (NOT disowned)
#     #### Glove
#     python3 index_searcher.py --only "flat"  --threads 16 --indexes_dir glove >> results/glove/bf3host_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 16 --indexes_dir glove >> results/glove/bf3host_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 16 --indexes_dir glove >> results/glove/bf3host_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 16 --indexes_dir glove >> results/glove/bf3host_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 16 --indexes_dir glove >> results/glove/bf3host_hnsw.txt  2>>&1

#     python3 index_searcher.py --only "flat"  --threads 8  --indexes_dir glove >> results/glove/bf3host_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 8  --indexes_dir glove >> results/glove/bf3host_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 8  --indexes_dir glove >> results/glove/bf3host_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 8  --indexes_dir glove >> results/glove/bf3host_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 8  --indexes_dir glove >> results/glove/bf3host_hnsw.txt  2>>&1 

#     python3 index_searcher.py --only "flat"  --threads 4  --indexes_dir glove >> results/glove/bf3host_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 4  --indexes_dir glove >> results/glove/bf3host_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 4  --indexes_dir glove >> results/glove/bf3host_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 4  --indexes_dir glove >> results/glove/bf3host_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 4  --indexes_dir glove >> results/glove/bf3host_hnsw.txt  2>>&1 

#     python3 index_searcher.py --only "flat"  --threads 2  --indexes_dir glove >> results/glove/bf3host_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 2  --indexes_dir glove >> results/glove/bf3host_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 2  --indexes_dir glove >> results/glove/bf3host_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 2  --indexes_dir glove >> results/glove/bf3host_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 2  --indexes_dir glove >> results/glove/bf3host_hnsw.txt  2>>&1 

#     python3 index_searcher.py --only "flat"  --threads 1  --indexes_dir glove >> results/glove/bf3host_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 1  --indexes_dir glove >> results/glove/bf3host_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 1  --indexes_dir glove >> results/glove/bf3host_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 1  --indexes_dir glove >> results/glove/bf3host_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 1  --indexes_dir glove >> results/glove/bf3host_hnsw.txt  2>>&1

#     #### Fasttext
#     python3 index_searcher.py --only "flat"  --threads 16 --indexes_dir fasttext >> results/fasttext/bf3host_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 16 --indexes_dir fasttext >> results/fasttext/bf3host_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 16 --indexes_dir fasttext >> results/fasttext/bf3host_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 16 --indexes_dir fasttext >> results/fasttext/bf3host_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 16 --indexes_dir fasttext >> results/fasttext/bf3host_hnsw.txt  2>>&1

#     python3 index_searcher.py --only "flat"  --threads 8  --indexes_dir fasttext >> results/fasttext/bf3host_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 8  --indexes_dir fasttext >> results/fasttext/bf3host_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 8  --indexes_dir fasttext >> results/fasttext/bf3host_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 8  --indexes_dir fasttext >> results/fasttext/bf3host_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 8  --indexes_dir fasttext >> results/fasttext/bf3host_hnsw.txt  2>>&1
    
#     python3 index_searcher.py --only "flat"  --threads 4  --indexes_dir fasttext >> results/fasttext/bf3host_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 4  --indexes_dir fasttext >> results/fasttext/bf3host_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 4  --indexes_dir fasttext >> results/fasttext/bf3host_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 4  --indexes_dir fasttext >> results/fasttext/bf3host_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 4  --indexes_dir fasttext >> results/fasttext/bf3host_hnsw.txt  2>>&1
    
#     python3 index_searcher.py --only "flat"  --threads 2  --indexes_dir fasttext >> results/fasttext/bf3host_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 2  --indexes_dir fasttext >> results/fasttext/bf3host_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 2  --indexes_dir fasttext >> results/fasttext/bf3host_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 2  --indexes_dir fasttext >> results/fasttext/bf3host_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 2  --indexes_dir fasttext >> results/fasttext/bf3host_hnsw.txt  2>>&1
    
#     python3 index_searcher.py --only "flat"  --threads 1  --indexes_dir fasttext >> results/fasttext/bf3host_flat.txt  2>>&1
#     python3 index_searcher.py --only "lsh"   --threads 1  --indexes_dir fasttext >> results/fasttext/bf3host_lsh.txt   2>>&1
#     python3 index_searcher.py --only "pq"    --threads 1  --indexes_dir fasttext >> results/fasttext/bf3host_pq.txt    2>>&1
#     python3 index_searcher.py --only "ivfpq" --threads 1  --indexes_dir fasttext >> results/fasttext/bf3host_ivfpq.txt 2>>&1
#     python3 index_searcher.py --only "hnsw"  --threads 1  --indexes_dir fasttext >> results/fasttext/bf3host_hnsw.txt  2>>&1

# fi