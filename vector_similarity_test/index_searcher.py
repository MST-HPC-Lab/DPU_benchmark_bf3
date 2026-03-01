# index_searcher.py

import argparse
import pandas as pd
import numpy as np
import faiss
from timeit import repeat
# import hnswlib  # SOPHIA EDIT: removed hnswlib, switching to FAISS HNSW
import json
import os

#sophia edit: refactored to load indices built by index_builder.py, and to run searches with timing and recall measurement, but without rebuilding indices (since that can be time consuming and we want to separate build vs. search time in our measurements)
from index_builder import (
    recall_at_k, batch_recall,
    #brute_force_build,
    brute_force_search,
    search, hnsw_search,
    #lsh_build, pq_build, ivfpq_build, hnsw_build,
    #test_build, test_search,
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

    # Load saved artifacts from builder

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

    # SOPHIA EDIT: Load numpy arrays directly (not DataFrames)
    # Builder now saves contiguous float32 numpy arrays.
    x_query_np = np.load(os.path.join(indices_dir, "x_query.npy"))
    x_train_np = np.load(os.path.join(indices_dir, "x_train.npy"))

    ib.x_query = x_query_np
    ib.x_train = x_train_np

    # Load FAISS indices

    ib.FL2   = faiss.read_index(os.path.join(indices_dir, "flat.index"))
    ib.LSH   = faiss.read_index(os.path.join(indices_dir, "lsh.index"))
    ib.PQ    = faiss.read_index(os.path.join(indices_dir, "pq.index"))
    ib.IVFPQ = faiss.read_index(os.path.join(indices_dir, "ivfpq.index"))

    # OLD HNSWLIB LOAD (COMMENTED OUT — DO NOT DELETE)
    # ib.HNSW = hnswlib.Index(space='l2', dim=ib.d)
    # ib.HNSW.load_index(os.path.join(indices_dir, "hnsw.bin"))
    # ib.HNSW.set_ef(ib.HNSW_efsearch)

    # SOPHIA EDIT: Load FAISS HNSW index
    ib.HNSW = faiss.read_index(os.path.join(indices_dir, "hnsw.index"))

    # Set efSearch after loading (important for FAISS)
    ib.HNSW.hnsw.efSearch = max(int(ib.HNSW_efsearch), 1)

    outpath = args.mem_out if args.mem_out else None

    # Memory monitored query loop (optional)

    # with MemoryMonitor(role="query", outpath=outpath, interval=args.interval) as mem:
    #     mem.log("after_load_indices")

    print("Indexes loaded. Running searches...")

    k_values = [1, 2, 3, 5, 7, 10, 25, 50, 75, 100]

    bf_times = []
    lsh_recalls, lsh_times = [], []
    pq_recalls, pq_times = [], []
    ivfpq_recalls, ivfpq_times = [], []
    hnsw_recalls, hnsw_times = [], []

    for k in k_values:

        #Brute Force
        bf_time = np.mean(repeat(lambda: brute_force_search(k), repeat=3, number=1))
        bf_times.append(bf_time)

        #LSH
        lsh_recall = search(ib.LSH, k)
        lsh_time = np.mean(repeat(lambda: search(ib.LSH, k, measure_accuracy=False), repeat=3, number=1))
        lsh_recalls.append(lsh_recall)
        lsh_times.append(lsh_time)

        #PQ
        pq_recall = search(ib.PQ, k)
        pq_time = np.mean(repeat(lambda: search(ib.PQ, k, measure_accuracy=False), repeat=3, number=1))
        pq_recalls.append(pq_recall)
        pq_times.append(pq_time)

        #IVFPQ
        ivfpq_recall = search(ib.IVFPQ, k)
        ivfpq_time = np.mean(repeat(lambda: search(ib.IVFPQ, k, measure_accuracy=False), repeat=3, number=1))
        ivfpq_recalls.append(ivfpq_recall)
        ivfpq_times.append(ivfpq_time)

        #HNSW
        hnsw_recall = hnsw_search(k)
        hnsw_time = np.mean(repeat(lambda: hnsw_search(k, measure_accuracy=False), repeat=3, number=1))
        hnsw_recalls.append(hnsw_recall)
        hnsw_times.append(hnsw_time) 

#    results = []
#    for k in [1, 2, 3, 5, 7, 10, 25, 50, 75, 100]:
#        results.append(test_search(k=k, r=3, verbose=False))

    #     mem.log("queries_finished")
#    bf_time, lsh_recall, lsh_time, pq_recall, pq_time, ivfpq_recall, ivfpq_time, hnsw_recall, hnsw_time = zip(*results)

    print("k:", k_values)
    print("Brute Force Time:", bf_times)
    print("LSH Recall:", lsh_recalls)
    print("LSH Time:", lsh_times)
    print("PQ Recall:", pq_recalls)
    print("PQ Time:", pq_times)
    print("IVFPQ Recall:", ivfpq_recalls)
    print("IVFPQ Time:", ivfpq_times)
    print("HNSW Recall:", hnsw_recalls)
    print("HNSW Time:", hnsw_times)
    