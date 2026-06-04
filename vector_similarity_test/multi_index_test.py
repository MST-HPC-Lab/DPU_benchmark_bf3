#multi_index_test.py 
import pandas as pd
import numpy as np
import faiss
import argparse
from timeit import repeat
import os
import json
# from mat73 import loadmat
import h5py

import index_builder as ib
from device_utils import is_bluefield
from index_searcher import jsonable



# Timing
def avg_time(fn, reps=3): # No cache warmup here, but later reps will be warm, so this creates a mixture.
    fn()
    return np.mean(repeat(fn, number=1, repeat=reps))


# TEST
def test_suite(filename="glove.6B.200d.txt", only=None, k=10, r=3):
    if "../Data/" not in filename: path = f"../Data/{filename}"
    else: path = filename

    if path[-4:] == ".mat": # Load matlab file
        # mat_data = loadmat(path)['fea'].T
        with h5py.File(path, 'r') as f:
            mat_data = np.array(f['fea'])

        d = mat_data.shape[1]

        N = len(mat_data)
        start_i = 0 # 99
        step = 100 # Makes 1/step test set
        test_i = [i for i in range(start_i, N, step)] # 1% test set
        train_i = [i for i in range(N) if (i % step != start_i)] # rest is train set
        # test_i = [i for i in range(d-1, N, d)]
        # train_i = [i for i in range(N) if (i + d-1) % d]

    else: # Load CSV or txt
        # Load Data File (TODO: make this more flexible for different datasets)
        # df = pd.read_csv("../Data/glove.6B.200d.txt", sep=" ", quoting=3, header=None)
        # d = 300
        df = pd.read_csv(path, sep=" ", quoting=3, skiprows=1, header=None)
        # Split into vocab column and data
        # vocab = df.iloc[:, 0]
        df = df.drop(0, axis=1)

        d = df.shape[1]

        # Fixed Split
        N = len(df)
        start_i = 0 # 99
        step = 100 # Makes 1/step test set
        test_i = [i for i in range(start_i, N, step)] # 1% test set
        train_i = [i for i in range(N) if (i % step != start_i)] # rest is train set
        # test_i = [i for i in range(d-1, N, d)]
        # train_i = [i for i in range(N) if (i + d-1) % d]

        ib.x_query = df.iloc[test_i]
        ib.x_train = df.iloc[train_i]
        del df
    
    # sharing these variables with index_builder.py
    ib.d = d
    # ib.k = k

    ib.x_query = np.ascontiguousarray(ib.x_query.to_numpy(dtype=np.float32))
    ib.x_train = np.ascontiguousarray(ib.x_train.to_numpy(dtype=np.float32))

    # Load ground truth answers from proper index
    if args.indexes_dir == "unknown":
        index_name = "glove" if "glove" in filename else "fasttext" if "fasttext" in filename else "sift" if "sift" in filename else "unknown"
        if index_name == "unknown": raise ValueError("Index name could not be determined from filename. Please specify.")    
    else:
        index_name = args.indexes_dir
    truth_path = f"indexes/{index_name}/truth_I,D.json"

    if only is None:
        only = {"flat", "lsh", "pq", "ivfpq", "hnsw", "hnsw_pq", "hnsw_sq"} #"bf", 
    else:
        only = set(only)

    assert d == 200 or d == 300, "This test suite is designed for d=200 or d=300. Please adjust pq_m_vals accordingly if using a different dimension. They must be factors of d."
    
    pq_m_vals = [4, 5, 10, 20, 40] if d == 200 else [4, 5, 10, 25, 30, 50] if d == 300 else None # "subquantizers" or "m" in the PQ index, which is the number of subvectors the original vector is split into. It must be a factor of d.
    pq_n_bits = [4, 6, 8] # "nbits_per_index" or "nbits" in the PQ index, which is the number of bits used to encode each subvector.

    nlist = [int(4 * np.sqrt(len(ib.x_train))), int(8 * np.sqrt(len(ib.x_train))), int(16 * np.sqrt(len(ib.x_train)))] # "nlist" or "ncentroids" in the IVFPQ index, which is the number of Voronoi cells (or clusters) used to partition the training data. It must be less than the number of training vectors.

    hnsw_m_vals = [8, 16, 32] # "M" in the HNSW index, which is the number of bi-directional links created for each new element during index construction. A higher M leads to higher recall but also higher search time and memory usage.
    hnsw_efc_vals = [128, 200] # "efConstruction" or "efc" in the HNSW index, which is the size of the dynamic list used during index construction to select neighbors. A higher efc leads to higher recall but also longer build time.
    hnsw_efs_vals = [128, 200] # "efSearch" or "efs" in the HNSW index, which is the size of the dynamic list used during search. A higher efs leads to higher recall but also longer search time. A good starting place is 2xk to 4xk, but it can be higher to improve recall.

    sq_vals = [faiss.ScalarQuantizer.QT_4bit, faiss.ScalarQuantizer.QT_8bit]

    print(f"(All times averaged over {r} repeats)")
    ib.load_truth(truth_path)

    current_results = { # To be saved as JSON at the end; structured as results["multitest"][device][dataset_filename][k] = current_results
        "date": pd.Timestamp.now().isoformat(),
        "repeats": r,
        "threads": str(ib.threads) if ib.threads is not None else "default",
        "dimensions": d,
        "num_vecs": len(ib.x_train) + len(ib.x_query),
        "train_size": len(ib.x_train),
        "test_size": len(ib.x_query),
        "cache": "mixture", # "warm" or "cold" or "mixture" (if some builds are warm and some are cold)
        "pq_m_vals": pq_m_vals,
        "pq_n_bits": pq_n_bits,
        "nlist": nlist,
        "hnsw_m_vals": hnsw_m_vals,
        "hnsw_efc_vals": hnsw_efc_vals,
        "hnsw_efs_vals": hnsw_efs_vals,
        "sq_vals": ["QT_4bit", "QT_8bit"]
    }

    # Ground Truth or Brute Force
    # if "bf" in only:
    #     print("\nBrute Force")
    #     # bf_build = avg_time(lambda: ib.brute_force_build(x_train), r)
    #     # brute_force_build()
    #     st = avg_time(lambda: ib.brute_force_search(k, measure_accuracy=False), r)
    #     # print("Build Time:", bf_build)
    #     # print("Search Time:", bf_search)
    #     # ib.x_train = None # Needed for other builds
    #     current_results["bf_recalls"] = 1.0 # By definition, brute force has perfect recall
    #     current_results["bf_times"] = st

    # Flat (Ground Truth)
    if "flat" in only:
        print("\nFlat (Brute Force)")
        ib.flat_build(ib.x_train) # bt = avg_time(lambda: ib.flat_build(ib.x_train), r)

        # rc = ib.search(ib.FL2, k)
        st = avg_time(lambda: ib.search(ib.FL2, k, measure_accuracy=False), r)
        # print("Build Time:", bt)
        # print("Search Time:", flat_search)
        ib.FL2 = None
        current_results["flat_recalls"] = 1.0 # By definition, flat brute force has perfect recall
        current_results["flat_times"] = st

    # LSH 
    if "lsh" in only:    
        print("\nLSH")
        nbits_vals = [32, 64, 128, 256, 512, 1024, 1600]
        recalls = np.ones(len(nbits_vals)) * -1
        times = np.ones(len(nbits_vals)) * -1
        for i, nbits in enumerate(nbits_vals):
            ib.lsh_build(ib.x_train, nbits) # bt = avg_time(lambda: ib.lsh_build(ib.x_train, nbits), r)
            # lsh_build(nbits)
            # brute_force_search(k)

            rc = ib.search(ib.LSH, k)
            st = avg_time(lambda: ib.search(ib.LSH, k, measure_accuracy=False), r)
            # print(f"nbits={nbits} | recall={rc:.3f}, time={st:.4f}")
            recalls[i] = rc
            times[i] = st
            ib.LSH = None
        current_results["lsh_recalls"] = recalls.tolist()
        current_results["lsh_times"] = times.tolist()

    # PQ
    if "pq" in only:
        print("\nPQ")
        recalls = np.ones((len(pq_m_vals), len(pq_n_bits))) * -1
        times = np.ones((len(pq_m_vals), len(pq_n_bits))) * -1
        for i, m in enumerate(pq_m_vals):
            for j, nbits in enumerate(pq_n_bits):
                ib.pq_build(ib.x_train, m, nbits) # bt = avg_time(lambda: ib.pq_build(ib.x_train, m), r)
                # pq_build(m)
                # brute_force_search(k)

                rc = ib.search(ib.PQ, k)
                st = avg_time(lambda: ib.search(ib.PQ, k, measure_accuracy=False), r)
                # print(f"m={m}, nbits={nbits} | recall={rc:.3f}, time={st:.4f}")
                recalls[i, j] = rc
                times[i, j] = st
                ib.PQ = None
        current_results["pq_recalls"] = recalls.tolist()
        current_results["pq_times"] = times.tolist()

    # IVFPQ 
    if "ivfpq" in only:
        print("\nIVFPQ")
        recalls = np.ones((len(nlist), len(pq_m_vals))) * -1
        times = np.ones((len(nlist), len(pq_m_vals))) * -1
        for i, ncentroids in enumerate(nlist): # Usually ranges from 4xsqrt(|x_train|) to 16xsqrt(|x_train|).
            for j, code_size in enumerate(pq_m_vals): # code_size and m are same if n_bits is 8, since code_size = (m * n_bits) / 8
                ib.ivfpq_build(ib.x_train, ncentroids, code_size, n_bits=8) #bt = avg_time(lambda: ib.ivfpq_build(ib.x_train, ncentroids, code_size, n_bits=8), r)
                ib.IVFPQ.nprobe = 32
                # ivfpq_build(nlist)
                # brute_force_search(k)

                rc = ib.search(ib.IVFPQ, k)
                st = avg_time(lambda: ib.search(ib.IVFPQ, k, measure_accuracy=False), r)
                # print(f"nlist={ncentroids}, code_size={code_size} | recall={rc:.3f}, time={st:.4f}")               
                recalls[i, j] = rc
                times[i, j] = st
                ib.IVFPQ = None
        current_results["ivfpq_recalls"] = recalls.tolist()
        current_results["ivfpq_times"] = times.tolist()

    # HNSW 
    if "hnsw" in only:
        print("\nHNSW")
        recalls = np.ones((len(hnsw_m_vals), len(hnsw_efc_vals), len(hnsw_efs_vals))) * -1
        times = np.ones((len(hnsw_m_vals), len(hnsw_efc_vals), len(hnsw_efs_vals))) * -1
        for i, M in enumerate(hnsw_m_vals):
            for j, efc in enumerate(hnsw_efc_vals): # Default is 40, but users find this range is the sweet spot. Should be significantly higher than M.
                ib.hnsw_build(ib.x_train, d, efc, M, 128) # bt = avg_time(lambda: ib.hnsw_build(ib.x_train, d, efc, M, efs=100), r)
                for s, efs in enumerate(hnsw_efs_vals): # A good starting place is 2xk to 4xk, but it can be higher to improve recall.
                    ib.HNSW.hnsw.efSearch = efs
                    # hnsw_build(M, efc, efs)
                    # brute_force_search(k) # truth_I calculation no longer needed; loaded from truth_I,D.json

                    rc = ib.search(ib.HNSW, k)
                    st = avg_time(lambda: ib.search(ib.HNSW, k, measure_accuracy=False), r)
                    # print(f"M={M}, efc={efc}, efs={efs} | recall={rc:.3f}, time={st:.4f}")
                    recalls[i, j, s] = rc
                    times[i, j, s] = st
                ib.HNSW = None
        current_results["hnsw_recalls"] = recalls.tolist()
        current_results["hnsw_times"] = times.tolist()

    # HNSW+PQ
    if "hnsw_pq" in only:
        print("\nHNSW+PQ")
        recalls = np.ones((len(hnsw_m_vals), len(hnsw_efc_vals), len(pq_m_vals), len(hnsw_efs_vals))) * -1
        times = np.ones((len(hnsw_m_vals), len(hnsw_efc_vals), len(pq_m_vals), len(hnsw_efs_vals))) * -1
        for i, M in enumerate(hnsw_m_vals):
            for j, efc in enumerate(hnsw_efc_vals): # Default is 40, but users find this range is the sweet spot. Should be significantly higher than M.
                for m, pq_m in enumerate(pq_m_vals):
                    ib.hnsw_pq_build(ib.x_train, d, efc, M, pq_m, 128) #bt = avg_time(lambda: ib.hnsw_pq_build(ib.x_train, M, efc, efs), r)
                    for s, efs in enumerate(hnsw_efs_vals): # A good starting place is 2xk to 4xk, but it can be higher to improve recall.
                        ib.HNSWPQ.hnsw.efSearch = efs
                        # hnsw_build(M, efc, efs)
                        # brute_force_search(k)

                        rc = ib.search(ib.HNSWPQ, k)
                        st = avg_time(lambda: ib.search(ib.HNSWPQ, k, measure_accuracy=False), r)
                        # print(f"M={M}, efc={efc}, efs={efs}, pq_m={pq_m} | recall={rc:.3f}, time={st:.4f}")
                        recalls[i, j, m, s] = rc
                        times[i, j, m, s] = st
                    ib.HNSWPQ = None
        current_results["hnsw_pq_recalls"] = recalls.tolist()
        current_results["hnsw_pq_times"] = times.tolist()

    # HNSW+SQ
    if "hnsw_sq" in only:
        print("\nHNSW+SQ")
        recalls = np.ones((len(hnsw_m_vals), len(hnsw_efc_vals), len(sq_vals), len(hnsw_efs_vals))) * -1
        times = np.ones((len(hnsw_m_vals), len(hnsw_efc_vals), len(sq_vals), len(hnsw_efs_vals))) * -1
        for i, M in enumerate(hnsw_m_vals):
            for j, efc in enumerate(hnsw_efc_vals): # Default is 40, but users find this range is the sweet spot. Should be significantly higher than M.
                for q, sq in enumerate(sq_vals):
                    ib.hnsw_sq_build(ib.x_train, d, efc, M, sq, 128) #bt = avg_time(lambda: ib.hnsw_sq_build(ib.x_train, M, efc, efs), r)
                    for s, efs in enumerate(hnsw_efs_vals): # A good starting place is 2xk to 4xk, but it can be higher to improve recall.
                        ib.HNSWSQ.hnsw.efSearch = efs
                        # hnsw_sq_build(M, efc, efs)
                        # brute_force_search(k)

                        rc = ib.search(ib.HNSWSQ, k)
                        st = avg_time(lambda: ib.search(ib.HNSWSQ, k, measure_accuracy=False), r)
                        # print(f"M={M}, efc={efc}, efs={efs}, sq_type={sq} | recall={rc:.3f}, time={st:.4f}")
                        recalls[i, j, q, s] = rc
                        times[i, j, q, s] = st
                    ib.HNSWSQ = None
        current_results["hnsw_sq_recalls"] = recalls.tolist()
        current_results["hnsw_sq_times"] = times.tolist()

    # Load existing results JSON file
    results_path = os.path.join("results", "results.json")
    if os.path.exists(results_path):
        with open(results_path, "r") as f:
            results = json.load(f) #, object_hook=lambda d: SimpleNamespace(**d))
    else:
        results = {}

    device = "bf3" if is_bluefield() else "host"

    # Find location in data structure
    if "multitest" not in results:
        results["multitest"] = {}
    if device not in results["multitest"]:
        results["multitest"][device] = {}
    if filename not in results["multitest"][device]:
        results["multitest"][device][filename] = {}
    if k not in results["multitest"][device][filename]:
        results["multitest"][device][filename][k] = {}
    slot = results["multitest"][device][filename][k]
    # Any runs modifying other values than these will be rewritten!

    for key, value in current_results.items():
        if key in slot and slot[key] != value:
            print(f"WARNING: Overwriting existing results for {key} in slot. Old value: {slot[key]}, new value: {value}")
        slot[key] = jsonable(value) # Convert any non-JSON-serializable values to JSON-serializable format (e.g. numpy arrays to lists)

    # Save results to JSON
    with open(results_path, "w") as f:
        json.dump(results, f, indent=4)

    return



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--file", type=str, default="glove.6B.200d.txt")
    parser.add_argument(
        "--only",
        nargs="+",
        default=None,
        help="Run only selected tests out of: flat lsh pq ivfpq hnsw hnsw_pq hnsw_sq"
    )
    parser.add_argument("--k", type=int, default=10)
    parser.add_argument("--r", type=int, default=3)
    parser.add_argument("--indexes_dir", type=str, default="unknown",
                        help="Directory containing indexes and metadata")
    args = parser.parse_args()

    test_suite(filename=args.file, only=args.only, k=args.k, r=args.r)



#--------- OLD CODE; now using imports from index_builder instead -------------#


# # Recall
# def k_recall_at_k(test, truth, k=k):
#     return len(set(truth).intersection(test)) / k

# def batch_recall(test_I, truth_I_k, k=k):
#     return np.mean([k_recall_at_k(row, truth_I_k[i], k) for i, row in enumerate(test_I)])


# # Flat
# FLAT = None

# def brute_force_build():
#     global FLAT
#     FLAT = faiss.IndexFlatL2(d)
#     FLAT.add(x_train)

# def brute_force_search(k=k):
#     global truth_D, truth_I
#     truth_D, truth_I = FLAT.search(x_query, k)

# # HNSW
# HNSW = None

# def hnsw_build(M=16, efc=200, efs=200):
#     global HNSW
#     HNSW = faiss.IndexHNSWFlat(d, M)
#     HNSW.hnsw.efConstruction = efc
#     HNSW.add(x_train)
#     HNSW.hnsw.efSearch = max(int(efs), 1)

# def hnsw_search(k=k, measure_accuracy=True):
#     D, I = HNSW.search(x_query, k)
#     if measure_accuracy:
#         return batch_recall(I, truth_I, k)
#     return I, D

# # LSH 
# LSH = None

# def lsh_build(nbits=256):
#     global LSH
#     LSH = faiss.IndexLSH(d, nbits)
#     LSH.add(x_train)

# def lsh_search(k=k, measure_accuracy=True):
#     D, I = LSH.search(x_query, k)
#     if measure_accuracy:
#         return batch_recall(I, truth_I, k)
#     return I, D

# # PQ 
# PQ = None

# def pq_build(m=8, nbits=8):
#     global PQ
#     PQ = faiss.IndexPQ(d, m, nbits)
#     PQ.train(x_train)
#     PQ.add(x_train)

# def pq_search(k=k, measure_accuracy=True):
#     D, I = PQ.search(x_query, k)
#     if measure_accuracy:
#         return batch_recall(I, truth_I, k)
#     return I, D

# # IVFPQ
# IVFPQ = None

# def ivfpq_build(nlist=100, m=8, nbits=8):
#     global IVFPQ
#     quantizer = faiss.IndexFlatL2(d)
#     IVFPQ = faiss.IndexIVFPQ(quantizer, d, nlist, m, nbits)
#     IVFPQ.train(x_train)
#     IVFPQ.add(x_train)

# def ivfpq_search(k=k, measure_accuracy=True):
#     D, I = IVFPQ.search(x_query, k)
#     if measure_accuracy:
#         return batch_recall(I, truth_I, k)
#     return I, D
