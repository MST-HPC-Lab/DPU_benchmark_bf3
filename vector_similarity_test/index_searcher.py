# index_searcher.py
import argparse
import pandas as pd
import numpy as np
import faiss
from timeit import repeat
import hnswlib
import json
import os

from index_builder import (
    recall_at_k, batch_recall,
    brute_force_build, brute_force_search,
    search, hnsw_search,
    lsh_build, pq_build, ivfpq_build, hnsw_build,
    test_build, test_search,
)

import index_builder as ib
from mem_utils import MemoryMonitor
os.makedirs("results", exist_ok=True)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--mem_out",
        type=str,
        default=None,
        help="Path to write memory CSV for query session",
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=0.5,
        help="Memory sampling interval seconds",
    )
    parser.add_argument(
        "--indices_dir",
        type=str,
        default="indices",
        help="Directory containing indices and meta.json (default: indices)",
    )
    args = parser.parse_args()

    indices_dir = args.indices_dir

    # load saved artifacts from builder
    meta_path = os.path.join(indices_dir, "meta.json")
    with open(meta_path, "r") as f:
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

    x_query_np = np.load(os.path.join(indices_dir, "x_query.npy"))
    x_train_np = np.load(os.path.join(indices_dir, "x_train.npy"))
    ib.x_query = pd.DataFrame(x_query_np)
    ib.x_train = pd.DataFrame(x_train_np)

    ib.FL2   = faiss.read_index(os.path.join(indices_dir, "flat.index"))
    ib.LSH   = faiss.read_index(os.path.join(indices_dir, "lsh.index"))
    ib.PQ    = faiss.read_index(os.path.join(indices_dir, "pq.index"))
    ib.IVFPQ = faiss.read_index(os.path.join(indices_dir, "ivfpq.index"))

    ib.HNSW = hnswlib.Index(space='l2', dim=ib.d)
    ib.HNSW.load_index(os.path.join(indices_dir, "hnsw.bin"))
    ib.HNSW.set_ef(ib.HNSW_efsearch)

    outpath = args.mem_out if args.mem_out else None

    with MemoryMonitor(role="query", outpath=outpath, interval=args.interval) as mem:
        mem.log("after_load_indices")
        print("Indexes loaded. Running searches...")

        results = []
        for k in [1, 2, 3, 5, 7, 10, 25, 50, 75, 100]:
            mem.log(f"before_test_search_k{k}")
            results.append(test_search(k=k, r=3, verbose=False))
            mem.log(f"after_test_search_k{k}")

        mem.log("queries_finished")

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
