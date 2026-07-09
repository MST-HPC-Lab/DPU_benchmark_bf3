#!/bin/bash

# nohup ./update_ground_truth.sh &

echo "UPDATING glove GROUND TRUTH"
python3 index_searcher.py --only "none" --indexes_dir "glove"

echo "UPDATING fasttext GROUND TRUTH"
python3 index_searcher.py --only "none" --indexes_dir "fasttext"

echo "UPDATING sift GROUND TRUTH"
python3 index_searcher.py --only "none" --indexes_dir "sift"

echo "UPDATING equalized 128d 400k GROUND TRUTH"
python3 index_searcher.py --only "none" --indexes_dir "glove_128d_400k"
python3 index_searcher.py --only "none" --indexes_dir "fasttext_128d_400k"
python3 index_searcher.py --only "none" --indexes_dir "sift_128d_400k"

echo "UPDATING sift scaling GROUND TRUTH"
python3 index_searcher.py --only "none" --indexes_dir "sift_100k"
python3 index_searcher.py --only "none" --indexes_dir "sift_200k"
python3 index_searcher.py --only "none" --indexes_dir "sift_500k"
python3 index_searcher.py --only "none" --indexes_dir "sift_1m"
python3 index_searcher.py --only "none" --indexes_dir "sift_2m"
python3 index_searcher.py --only "none" --indexes_dir "sift_5m"
python3 index_searcher.py --only "none" --indexes_dir "sift_10m"