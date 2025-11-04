import argparse
import pandas as pd
import numpy as np
import faiss
from timeit import repeat
import hnswlib
import json

from index_builder import (
    # Recall Measurement functions
    recall_at_k, batch_recall,
    # Flat Index (Brute Force)
    brute_force_build, brute_force_search,
    # Search function is used for all other indexes
    search, hnsw_search,
    # LSH Index
    lsh_build,
    # PQ Index
    pq_build,
    # IVF PQ Index
    ivfpq_build,
    # HNSW Index
    hnsw_build,
    # To build without timing it
    test_build,
    # To time the search only
    test_search,
)

import index_builder as ib  # set/load globals here


if __name__ == "__main__":
    # load saved artifacts from builder
    with open("indices/meta.json", "r") as f:
        meta = json.load(f)

    ib.d = int(meta["d"])
    ib.lsh_nbits = int(meta["lsh_nbits"])
    ib.pq_nbits = int(meta["pq_nbits"])
    ib.pq_subquantizers = int(meta["pq_subquantizers"])
    ib.ivfpq_codesize = int(meta["ivfpq_codesize"])
    ib.ivfpq_ncentroids = int(meta["ivfpq_ncentroids"])
    ib.HNSW_M = int(meta["HNSW_M"])
    ib.HNSW_efconstruction = int(meta["HNSW_efconstruction"])
    ib.HNSW_efsearch = int(meta["HNSW_efsearch"])

    x_query_np = np.load("indices/x_query.npy")
    x_train_np = np.load("indices/x_train.npy")
    ib.x_query = pd.DataFrame(x_query_np)
    ib.x_train = pd.DataFrame(x_train_np)

    ib.FL2   = faiss.read_index("indices/flat.index")
    ib.LSH   = faiss.read_index("indices/lsh.index")
    ib.PQ    = faiss.read_index("indices/pq.index")
    ib.IVFPQ = faiss.read_index("indices/ivfpq.index")

    ib.HNSW = hnswlib.Index(space='l2', dim=ib.d)
    ib.HNSW.load_index("indices/hnsw.bin")
    ib.HNSW.set_ef(ib.HNSW_efsearch)

    print("Indexes loaded. Running searches...")

    results = []
    for k in [1, 2, 3, 5, 7, 10, 25, 50, 75, 100]:
        results.append(test_search(k=k, r=3, verbose=False))
    bf_time, lsh_recall, lsh_time, pq_recall, pq_time, ivfpq_recall, ivfpq_time, hnsw_recall, hnsw_time = zip(*results)

    print("Brute Force Time:", bf_time)
    print("LSH Recall:", lsh_recall)
    print("LSH Time:", lsh_time)
    print("PQ Recall:", pq_recall)
    print("PQ Time:", pq_time)
    print("IVFPQ Recall:", ivfpq_recall)
    print("IVFPQ Time:", ivfpq_time)
    print("HNSW Recall:", hnsw_recall)
    print("HNSW Time:", hnsw_time)
