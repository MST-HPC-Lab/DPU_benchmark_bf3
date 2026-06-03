# index_searcher.py
import os
import argparse
import pandas as pd
from timeit import repeat
# import hnswlib  # removed hnswlib, now using FAISS HNSW
import json

from device_utils import is_bluefield



def jsonable(x):
    """Convert numpy values to normal Python values so json.dump will work."""
    if isinstance(x, np.ndarray):
        return x.tolist()
    if isinstance(x, np.integer):
        return int(x)
    if isinstance(x, np.floating):
        return float(x)
    if isinstance(x, list):
        return [jsonable(v) for v in x]
    if isinstance(x, tuple):
        return [jsonable(v) for v in x]
    if isinstance(x, dict):
        return {str(k): jsonable(v) for k, v in x.items()}
    return x


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
        "--indexes_dir",
        type=str,
        default="glove",
        help="Directory containing indexes and meta.json (default: glove)",
    )
    parser.add_argument(
        "--only",
        nargs="+",
        default=None,
        help="Run only selected indexes: flat lsh pq ivfpq hnsw"
    )
    parser.add_argument(
        "--threads",
        type=int,
        default=None,
        help="Limit the number of threads used by FAISS."
    )
    args = parser.parse_args()

    #---------------------- IMPORTS ---------------------------------#
    # Force thread counts BEFORE importing numpy/faiss
    # if args.threads is not None:
    #     os.environ["OMP_NUM_THREADS"] = args.threads
    #     os.environ["OPENBLAS_NUM_THREADS"] = args.threads
    #     os.environ["MKL_NUM_THREADS"] = args.threads
    #     os.environ["VECLIB_MAXIMUM_THREADS"] = args.threads
    #     os.environ["NUMEXPR_NUM_THREADS"] = args.threads
    #     import faiss
    #     faiss.omp_set_num_threads(int(args.threads)) 
    # else: import faiss

    if args.threads is not None:
        thread_count = str(args.threads)
        os.environ["OMP_NUM_THREADS"] = thread_count
        os.environ["OPENBLAS_NUM_THREADS"] = thread_count
        os.environ["MKL_NUM_THREADS"] = thread_count
        os.environ["VECLIB_MAXIMUM_THREADS"] = thread_count
        os.environ["NUMEXPR_NUM_THREADS"] = thread_count
        import faiss
        faiss.omp_set_num_threads(args.threads)
        # NOTE: Does not affect AVX or NEON (SMID capabilities)
    else:
        import faiss

    import numpy as np
    print("FAISS threads:", faiss.omp_get_max_threads()) 

    #sophia edit: refactored to load indexes built by index_builder.py, and to run searches with timing and recall measurement, but without rebuilding indexes (since that can be time consuming and we want to separate build vs. search time in our measurements)
    from index_builder import (
        brute_force_search,
        search, #hnsw_search,
        #test_search,
    )
    import index_builder as ib
    from mem_utils import MemoryMonitor


    #---------------------- SETUP ---------------------------------#
    REPLICATIONS = 5
    os.makedirs("results", exist_ok=True)

    #warmup function to ensure fair timing (e.g. JIT compilation, caching effects)
    def avg_time(fn, reps=REPLICATIONS, warmup_runs=2):
        for _ in range(warmup_runs): #extra runs to warm up caches 
            fn()
        #fn()
        return np.mean(repeat(fn, repeat=reps, number=1))

    only = set(args.only) if args.only is not None else {"flat", "lsh", "pq", "ivfpq", "hnsw", "hnsw_pq", "hnsw_sq"} # "bf", 
    # if "brute" in only:
    #     only.add("bf")
    if "hnswpq" in only:
        only.add("hnsw_pq")
    if "hnswsq" in only:
        only.add("hnsw_sq")
    indexes_dir = os.path.join("indexes", args.indexes_dir) if "indexes" not in args.indexes_dir else args.indexes_dir

    # Load saved artifacts from builder
    meta_path = os.path.join(indexes_dir, "meta.json")
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
    ib.HNSW_SQ_qtype = int(meta["HNSW_SQ_qtype"])

    # SOPHIA EDIT: Load numpy arrays directly (not DataFrames)
    # Builder now saves contiguous float32 numpy arrays.
    ib.x_query = np.load(os.path.join(indexes_dir, "x_query.npy"))
    #x_train_np = np.load(os.path.join(indexes_dir, "x_train.npy"))
    #ib.x_train = x_train_np

    # ib.FL2   = faiss.read_index(os.path.join(indexes_dir, "flat.index"))
    # ib.LSH   = faiss.read_index(os.path.join(indexes_dir, "lsh.index"))
    # ib.PQ    = faiss.read_index(os.path.join(indexes_dir, "pq.index"))
    # ib.IVFPQ = faiss.read_index(os.path.join(indexes_dir, "ivfpq.index")) 

    # OLD HNSWLIB LOAD (COMMENTED OUT — DO NOT DELETE) 
    # ib.HNSW = hnswlib.Index(space='l2', dim=ib.d)
    # ib.HNSW.load_index(os.path.join(indexes_dir, "hnsw.bin"))
    # ib.HNSW.set_ef(ib.HNSW_efsearch)

    # SOPHIA EDIT: Load FAISS HNSW index
    #ib.HNSW = faiss.read_index(os.path.join(indexes_dir, "hnsw.index"))

    # Set efSearch after loading (important for FAISS)
    #ib.HNSW.hnsw.efSearch = max(int(ib.HNSW_efsearch), 1)

    outpath = args.mem_out if args.mem_out else None

    # Memory monitored query loop (optional)

    # with MemoryMonitor(role="query", outpath=outpath, interval=args.interval) as mem:
    #     mem.log("after_load_indexes")

    print("Indexes loaded. Running searches...", flush=True)

    # bf_times = []
    flat_times = [] # flat_recalls = []
    lsh_recalls, lsh_times = [], []
    pq_recalls, pq_times = [], []
    ivfpq_recalls, ivfpq_times = [], []
    hnsw_recalls, hnsw_times = [], []
    hnsw_pq_recalls, hnsw_pq_times = [], []
    hnsw_sq_recalls, hnsw_sq_times = [], []

    print("Threads:", args.threads, flush=True)
    print("k:", ib.k_values, flush=True)

    # Load ground truth for recall calculations
    ib.load_truth(os.path.join(indexes_dir, "truth_I,D.json")) # Load ground truth for recall calculations

    # # Brute Force 
    # if "bf" in only:
    #     ib.x_train = np.load(os.path.join(indexes_dir, "x_train.npy"))
    #     for k in ib.k_values:
    #         # brute_force_search(k)
    #         bf_time = avg_time(lambda: brute_force_search(k, measure_accuracy=False))
    #         bf_times.append(bf_time)
        
    #     print("Brute Force Time:", bf_times, flush=True)
    #     ib.x_train = None # free memory

    # Flat
    if "flat" in only:
        ib.FL2 = faiss.read_index(os.path.join(indexes_dir, "flat.index"))
        for k in ib.k_values:
            # brute_force_search(k)  # keep truth current / consistent
            # flat_recall = search(ib.FL2, k)
            flat_time = avg_time(lambda: search(ib.FL2, k, measure_accuracy=False))
            # flat_recalls.append(flat_recall)
            flat_times.append(flat_time)

        # print("Flat Recall:", flat_recalls, flush=True)
        print("Flat Time:", flat_times, flush=True)
        ib.FL2 = None

    #  LSH
    if "lsh" in only:
        ib.LSH = faiss.read_index(os.path.join(indexes_dir, "lsh.index"))
        for k in ib.k_values:
            # brute_force_search(k)
            lsh_recall = search(ib.LSH, k)
            lsh_time = avg_time(lambda: search(ib.LSH, k, measure_accuracy=False))
            lsh_recalls.append(lsh_recall)
            lsh_times.append(lsh_time)

        print("LSH Recall:", lsh_recalls, flush=True)
        print("LSH Time:", lsh_times, flush=True)
        ib.LSH = None

    # PQ 
    if "pq" in only:
        ib.PQ = faiss.read_index(os.path.join(indexes_dir, "pq.index"))
        for k in ib.k_values:
            # brute_force_search(k)
            pq_recall = search(ib.PQ, k)
            pq_time = avg_time(lambda: search(ib.PQ, k, measure_accuracy=False))
            pq_recalls.append(pq_recall)
            pq_times.append(pq_time)

        print("PQ Recall:", pq_recalls, flush=True)
        print("PQ Time:", pq_times, flush=True)
        ib.PQ = None

    #  IVFPQ
    if "ivfpq" in only:
        ib.IVFPQ = faiss.read_index(os.path.join(indexes_dir, "ivfpq.index"))
        for k in ib.k_values:
            # brute_force_search(k)
            ivfpq_recall = search(ib.IVFPQ, k)
            ivfpq_time = avg_time(lambda: search(ib.IVFPQ, k, measure_accuracy=False))
            ivfpq_recalls.append(ivfpq_recall)
            ivfpq_times.append(ivfpq_time)

        print("IVFPQ Recall:", ivfpq_recalls, flush=True)
        print("IVFPQ Time:", ivfpq_times, flush=True)
        ib.IVFPQ = None

    #  HNSW
    if "hnsw" in only:
        ib.HNSW = faiss.read_index(os.path.join(indexes_dir, "hnsw.index"))
        ib.HNSW.hnsw.efSearch = max(int(ib.HNSW_efsearch),1)
        for k in ib.k_values:
            # brute_force_search(k)
            hnsw_recall = search(ib.HNSW, k)
            hnsw_time = avg_time(lambda: search(ib.HNSW, k, measure_accuracy=False))
            # hnsw_recall = hnsw_search(k)
            # hnsw_time = avg_time(lambda: hnsw_search(k, measure_accuracy=False))
            hnsw_recalls.append(hnsw_recall)
            hnsw_times.append(hnsw_time)

        print("HNSW Recall:", hnsw_recalls, flush=True)
        print("HNSW Time:", hnsw_times, flush=True)
        ib.HNSW = None

    # HNSW + PQ
    if "hnsw_pq" in only:
        ib.HNSWPQ = faiss.read_index(os.path.join(indexes_dir, "hnsw_pq.index"))
        ib.HNSWPQ.hnsw.efSearch = max(int(ib.HNSW_efsearch),1)
        for k in ib.k_values:
            # brute_force_search(k)
            hnsw_pq_recall = search(ib.HNSWPQ, k)
            hnsw_pq_time = avg_time(lambda: search(ib.HNSWPQ, k, measure_accuracy=False))
            hnsw_pq_recalls.append(hnsw_pq_recall)
            hnsw_pq_times.append(hnsw_pq_time)

        print("HNSW+PQ Recall:", hnsw_pq_recalls, flush=True)
        print("HNSW+PQ Time:", hnsw_pq_times, flush=True)
        ib.HNSWPQ = None

    # HNSW + SQ
    if "hnsw_sq" in only:
        ib.HNSWSQ = faiss.read_index(os.path.join(indexes_dir, "hnsw_sq.index"))
        ib.HNSWSQ.hnsw.efSearch = max(int(ib.HNSW_efsearch),1)
        for k in ib.k_values:
            # brute_force_search(k)
            hnsw_sq_recall = search(ib.HNSWSQ, k)
            hnsw_sq_time = avg_time(lambda: search(ib.HNSWSQ, k, measure_accuracy=False))
            hnsw_sq_recalls.append(hnsw_sq_recall)
            hnsw_sq_times.append(hnsw_sq_time)

        print("HNSW+SQ Recall:", hnsw_sq_recalls, flush=True)
        print("HNSW+SQ Time:", hnsw_sq_times, flush=True)
        ib.HNSWSQ = None


        
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

    # Gather current results and insert into loaded JSON
    device = "bf3" if is_bluefield() else "host"
    dataset = str(meta["source_file"])
    dimensions = str(meta["d"])
    num_vecs = str(meta["num_vecs"])
    threads = str(args.threads) if args.threads is not None else "default"


    if device not in results:
        results[device] = {}

    if dataset not in results[device]:
        results[device][dataset] = {}

    if dimensions not in results[device][dataset]:
        results[device][dataset][dimensions] = {}

    if num_vecs not in results[device][dataset][dimensions]:
        results[device][dataset][dimensions][num_vecs] = {}

    if threads not in results[device][dataset][dimensions][num_vecs]:
        results[device][dataset][dimensions][num_vecs][threads] = {}

    slot = results[device][dataset][dimensions][num_vecs][threads]

    old_date = slot.get("date", None) # Will be None if slot is empty
    if old_date is not None: # Slot was not empty, preserve old results
        old_date_slot = {}
        slot[old_date] = old_date_slot

    current_results = { # To become the JSON dict entry for this run
        "date": pd.Timestamp.now().isoformat(),
        "repeats": REPLICATIONS,
        "train_size": num_vecs - len(ib.x_query),
        "query_size": len(ib.x_query),
        "k_values": ib.k_values,

        # "bf_times": bf_times,
        # "flat_recalls": flat_recalls,
        "flat_times": flat_times,
        "lsh_recalls": lsh_recalls,
        "lsh_times": lsh_times,
        "pq_recalls": pq_recalls,
        "pq_times": pq_times,
        "ivfpq_recalls": ivfpq_recalls,
        "ivfpq_times": ivfpq_times,
        "hnsw_recalls": hnsw_recalls,
        "hnsw_times": hnsw_times,
        "hnsw_pq_recalls": hnsw_pq_recalls,
        "hnsw_pq_times": hnsw_pq_times,
        "hnsw_sq_recalls": hnsw_sq_recalls,
        "hnsw_sq_times": hnsw_sq_times,
    }

    slot["date"] = current_results["date"]

    for key, value in current_results.items():
        if key == "date":
            continue

        if value is None:
            continue

        if isinstance(value, (list, tuple, dict, np.ndarray)) and len(value) == 0:
            continue  # skip empty result arrays/lists only

        old_item = slot.get(key, None)
        if old_item is not None and old_date is not None:
            old_date_slot[key] = old_item

        slot[key] = jsonable(value)

    # Save results to JSON
    with open(results_path, "w") as f:
        json.dump(results, f, indent=4)
