#!/bin/bash

# nohup ./multitest_suite.sh &

# echo "RUNNING glove MULTI-INDEX TESTS WITH k=10"
# python3 multi_index_test.py --file "glove.6B.200d.txt" --k 10 --only "flat"
# python3 multi_index_test.py --file "glove.6B.200d.txt" --k 10 --only "lsh"
# python3 multi_index_test.py --file "glove.6B.200d.txt" --k 10 --only "pq"
# python3 multi_index_test.py --file "glove.6B.200d.txt" --k 10 --only "ivfpq"
# python3 multi_index_test.py --file "glove.6B.200d.txt" --k 10 --only "hnsw"
# python3 multi_index_test.py --file "glove.6B.200d.txt" --k 10 --only "hnsw_pq"
# python3 multi_index_test.py --file "glove.6B.200d.txt" --k 10 --only "hnsw_sq"

# echo "RUNNING fasttext MULTI-INDEX TESTS WITH k=10"
# python3 multi_index_test.py --file "fasttext_cc.en.300.vec" --k 10 --only "flat"
# python3 multi_index_test.py --file "fasttext_cc.en.300.vec" --k 10 --only "lsh"
# python3 multi_index_test.py --file "fasttext_cc.en.300.vec" --k 10 --only "pq"
# python3 multi_index_test.py --file "fasttext_cc.en.300.vec" --k 10 --only "ivfpq"
# python3 multi_index_test.py --file "fasttext_cc.en.300.vec" --k 10 --only "hnsw"
# python3 multi_index_test.py --file "fasttext_cc.en.300.vec" --k 10 --only "hnsw_pq"
# python3 multi_index_test.py --file "fasttext_cc.en.300.vec" --k 10 --only "hnsw_sq"

echo "RUNNING sift MULTI-INDEX TESTS WITH k=10"
python3 multi_index_test.py --file "sift10m.mat" --k 10 --only "flat"
python3 multi_index_test.py --file "sift10m.mat" --k 10 --only "lsh"
python3 multi_index_test.py --file "sift10m.mat" --k 10 --only "pq"
python3 multi_index_test.py --file "sift10m.mat" --k 10 --only "ivfpq"
python3 multi_index_test.py --file "sift10m.mat" --k 10 --only "hnsw"
python3 multi_index_test.py --file "sift10m.mat" --k 10 --only "hnsw_pq"
python3 multi_index_test.py --file "sift10m.mat" --k 10 --only "hnsw_sq"