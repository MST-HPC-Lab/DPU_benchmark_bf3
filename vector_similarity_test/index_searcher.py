# index_searcher.py
import os

import argparse
import pandas as pd
from timeit import repeat
# import hnswlib  # SOPHIA EDIT: removed hnswlib, switching to FAISS HNSW
import json
from types import SimpleNamespace

from device_utils import is_bluefield


if __name__ == "__main__":
    #---------------------- PARAMETERS ---------------------------------#
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
    parser.add_argument(
        "--only",
        nargs="+",
        default=None,
        help="Run only selected indices: flat lsh pq ivfpq hnsw"
    )
    parser.add_argument(
        "--threads",
        nargs="+",
        default=None,
        help="Limit the number of threads used by FAISS."
    )
    args = parser.parse_args()

    #---------------------- IMPORTS ---------------------------------#
    # Force thread counts BEFORE importing numpy/faiss
    if args.threads is not None:
        os.environ["OMP_NUM_THREADS"] = args.threads
        os.environ["OPENBLAS_NUM_THREADS"] = args.threads
        os.environ["MKL_NUM_THREADS"] = args.threads
        os.environ["VECLIB_MAXIMUM_THREADS"] = args.threads
        os.environ["NUMEXPR_NUM_THREADS"] = args.threads
        import faiss
        faiss.omp_set_num_threads(int(args.threads)) 
    else: import faiss
    import numpy as np
    print("FAISS threads:", faiss.omp_get_max_threads()) 

    #sophia edit: refactored to load indices built by index_builder.py, and to run searches with timing and recall measurement, but without rebuilding indices (since that can be time consuming and we want to separate build vs. search time in our measurements)
    from index_builder import (
        brute_force_search,
        search, hnsw_search,
        #test_search,
    )
    import index_builder as ib
    from mem_utils import MemoryMonitor


    #---------------------- SETUP ---------------------------------#
    REPLICATIONS = 5
    os.makedirs("results", exist_ok=True)

    #warmup function to ensure fair timing (e.g. JIT compilation, caching effects)
    def avg_time(fn, reps=REPLICATIONS):
        for _ in range(2): #two extra runs to warm up caches 
            fn()
        #fn()
        return np.mean(repeat(fn, repeat=reps, number=1))

    only = set(args.only) if args.only is not None else {"flat", "lsh", "pq", "ivfpq", "hnsw"}
    
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

    ib._x_train_np = x_train_np  # for brute force search without FAISS

    # Load FAISS indices

    ib.FL2 = faiss.read_index(os.path.join(indices_dir, "flat.index"))

    if "lsh" in only:
        ib.LSH = faiss.read_index(os.path.join(indices_dir, "lsh.index"))
    
    if "pq" in only:
        ib.PQ = faiss.read_index(os.path.join(indices_dir, "pq.index"))

    if "ivfpq" in only:
        ib.IVFPQ = faiss.read_index(os.path.join(indices_dir, "ivfpq.index"))

    if "hnsw" in only:
        ib.HNSW = faiss.read_index(os.path.join(indices_dir, "hnsw.index"))
        ib.HNSW.hnsw.efSearch = max(int(ib.HNSW_efsearch),1)
        

    # ib.FL2   = faiss.read_index(os.path.join(indices_dir, "flat.index"))
    # ib.LSH   = faiss.read_index(os.path.join(indices_dir, "lsh.index"))
    # ib.PQ    = faiss.read_index(os.path.join(indices_dir, "pq.index"))
    # ib.IVFPQ = faiss.read_index(os.path.join(indices_dir, "ivfpq.index")) 

    # OLD HNSWLIB LOAD (COMMENTED OUT — DO NOT DELETE) 
    # ib.HNSW = hnswlib.Index(space='l2', dim=ib.d)
    # ib.HNSW.load_index(os.path.join(indices_dir, "hnsw.bin"))
    # ib.HNSW.set_ef(ib.HNSW_efsearch)

    # SOPHIA EDIT: Load FAISS HNSW index
    #ib.HNSW = faiss.read_index(os.path.join(indices_dir, "hnsw.index"))

    # Set efSearch after loading (important for FAISS)
    #ib.HNSW.hnsw.efSearch = max(int(ib.HNSW_efsearch), 1)

    outpath = args.mem_out if args.mem_out else None

    # Memory monitored query loop (optional)

    # with MemoryMonitor(role="query", outpath=outpath, interval=args.interval) as mem:
    #     mem.log("after_load_indices")

    print("Indexes loaded. Running searches...", flush=True)

    k_values = [1, 2, 3, 5, 7, 10, 25, 50, 75, 100]

    bf_times = [] #FAISS brute force (flat index) times 
    bf_times_np = [] #numpy brute force (no FAIS) 
    lsh_recalls, lsh_times = [], []
    pq_recalls, pq_times = [], []
    ivfpq_recalls, ivfpq_times = [], []
    hnsw_recalls, hnsw_times = [], []

    print("k:", k_values, flush=True)

    # Brute Force — FAISS
    if "flat" in only:
        for k in k_values:
            brute_force_search(k)
            bf_time = avg_time(lambda: ib.FL2.search(ib.x_query, k))
            bf_times.append(bf_time)
        print("Brute Force Time (FAISS):", bf_times, flush=True)

    # Brute Force — numpy (no FAISS)
    if "flat" in only:
        for k in k_values:
            brute_force_search(k)
            bf_time_np = avg_time(lambda: brute_force_search(k))
            bf_times_np.append(bf_time_np)
        print("Brute Force Time (no FAISS):", bf_times_np, flush=True)

    #  LSH
    if "lsh" in only:
        for k in k_values:
            brute_force_search(k)
            lsh_recall = search(ib.LSH, k)
            lsh_time = avg_time(lambda: search(ib.LSH, k, measure_accuracy=False))
            lsh_recalls.append(lsh_recall)
            lsh_times.append(lsh_time)

        print("LSH Recall:", lsh_recalls, flush=True)
        print("LSH Time:", lsh_times, flush=True)

    # PQ 
    if "pq" in only:
        for k in k_values:
            brute_force_search(k)
            pq_recall = search(ib.PQ, k)
            pq_time = avg_time(lambda: search(ib.PQ, k, measure_accuracy=False))
            pq_recalls.append(pq_recall)
            pq_times.append(pq_time)

        print("PQ Recall:", pq_recalls, flush=True)
        print("PQ Time:", pq_times, flush=True)

    #  IVFPQ
    if "ivfpq" in only:
        for k in k_values:
            brute_force_search(k)
            ivfpq_recall = search(ib.IVFPQ, k)
            ivfpq_time = avg_time(lambda: search(ib.IVFPQ, k, measure_accuracy=False))
            ivfpq_recalls.append(ivfpq_recall)
            ivfpq_times.append(ivfpq_time)

        print("IVFPQ Recall:", ivfpq_recalls, flush=True)
        print("IVFPQ Time:", ivfpq_times, flush=True)

    #  HNSW
    if "hnsw" in only:
        for k in k_values:
            brute_force_search(k)
            hnsw_recall = hnsw_search(k)
            hnsw_time = avg_time(lambda: hnsw_search(k, measure_accuracy=False))
            hnsw_recalls.append(hnsw_recall)
            hnsw_times.append(hnsw_time)

        print("HNSW Recall:", hnsw_recalls, flush=True)
        print("HNSW Time:", hnsw_times, flush=True)


        
    # results = []
    # for k in [1, 2, 3, 5, 7, 10, 25, 50, 75, 100]:
    #     results.append(test_search(k=k, r=3, verbose=False))

    #     mem.log("queries_finished")
    # bf_time, lsh_recall, lsh_time, pq_recall, pq_time, ivfpq_recall, ivfpq_time, hnsw_recall, hnsw_time = zip(*results)


    # Load Results JSON file
    results_path = os.path.join("results", "results.json")
    if os.path.exists(results_path):
        with open(results_path, "r") as f:
            results = json.load(f) #, object_hook=lambda d: SimpleNamespace(**d))
    else:
        results = {} #SimpleNamespace()

    # Gather current results and insert into loaded JSON (overwriting any existing results for this indices_dir)
    device = "bf3" if is_bluefield() else "host"
    dataset = meta["source_file"]
    dimensions = meta["d"]
    num_vecs = meta["num_vecs"]
    threads = args.threads

    if device not in results: results[device] = {}
    if dataset not in results[device]: results[device][dataset] = {}
    if dimensions not in results[device][dataset]: results[device][dataset][dimensions] = {}
    if num_vecs not in results[device][dataset][dimensions]: results[device][dataset][dimensions][num_vecs] = {}
    if threads not in results[device][dataset][dimensions][num_vecs]: results[device][dataset][dimensions][num_vecs][threads] = {}
    slot = results[device][dataset][dimensions][num_vecs][threads]
    old_date = slot.get("date", None)
    if old_date is not None:
        old_date_slot = slot.get(old_date, {})
        slot[old_date] = old_date_slot

    current_results = { # To add to JSON dict
        "date": pd.Timestamp.now().isoformat(),
        "repeats": REPLICATIONS,
        "k_values" : k_values,

        "bf_times" : bf_times, #faiss brute force times
        "bf_times_np" : bf_times_np, #no FAISS brute force times
        "lsh_recalls" : lsh_recalls,
        "lsh_times" : lsh_times,
        "pq_recalls" : pq_recalls,
        "pq_times" : pq_times,
        "ivfpq_recalls" : ivfpq_recalls,
        "ivfpq_times" : ivfpq_times,
        "hnsw_recalls" : hnsw_recalls,
        "hnsw_times" : hnsw_times,
    }
    for key, value in current_results.items():
        if key == "date": continue
        # Save old data under old date as subkey
        old_item = slot.get(key, None)
        if old_item is not None: old_date_slot[key] = old_item
        # Insert current data
        slot[key] = value


    # def serialize_namespace(obj):
    #     if isinstance(obj, SimpleNamespace):
    #         return vars(obj)
    #     raise TypeError(f"Type {type(obj)} not serializable")

    # Save results to JSON
    with open(results_path, "w") as f:
        json.dump(results, f, indent=4)