# index_builder.py
import argparse
from sys import argv
import pandas as pd
import numpy as np
import faiss
from timeit import repeat
# import hnswlib  # SOPHIA EDIT: removed hnswlib, switching to FAISS HNSW
import os, json
# from mat73 import loadmat
import h5py
import pickle

from mem_utils import MemoryMonitor
os.makedirs("results", exist_ok=True)

# Constants
k_values = [1, 2, 3, 5, 7, 10, 25, 50, 75, 100]

# Globals used across functions
truth_I = {} #truth_D = {}
x_train, x_query = None, None
FL2 = None
LSH = None
PQ = None
IVFPQ = None
HNSW = None
HNSWSQ = None
HNSWPQ = None

TRUTH_FILE_NAME = "truth_I.pkl"


# Recall helpers

def k_recall_at_k(test, truth, k):
    assert len(test) == k
    assert len(truth) == k
    return len(set(truth).intersection(test)) / k

def batch_recall(test_I, truth_I_k, k):
    # results = []
    # for i, row in enumerate(test_I):
    #     results.append(k_recall_at_k(row, truth_I_k[i], k))
    # return sum(results) / len(results)
    return np.mean([k_recall_at_k(row, truth_I_k[i], k) for i, row in enumerate(test_I)])


# def brute_force_build(data):
#     # global FL2
#     # global d
#     # Use FAISS flat but we'll override search with numpy anyway
#     # FL2 = faiss.IndexFlatL2(d)
#     # FL2.add(data)
#     # Store train data for numpy fallback
#     global _x_train_np
#     _x_train_np = data  # already float32 contiguous

# def brute_force_search(k, measure_accuracy=False):
#     global truth_D, truth_I, x_train
#     # Pure numpy brute force — no AVX/BLAS optimization
#     # Compute L2 distances manually
#     dists = np.sum((x_query[:, None, :] - x_train[None, :, :]) ** 2, axis=-1)  # (nq, n_train)
#     truth_I = np.argsort(dists, axis=1)[:, :k].astype(np.int64)
#     truth_D = np.sort(dists, axis=1)[:, :k]
#     if measure_accuracy:
#         return 1.0

# def brute_force_search(k, measure_accuracy=False):
#     global truth_D, truth_I, x_query, x_train
#     nq = x_query.shape[0]
#     all_I = np.empty((nq, k), dtype=np.int64)
#     all_D = np.empty((nq, k), dtype=np.float32)

#     BATCH = 100
#     for start in range(0, nq, BATCH):
#         end = min(start + BATCH, nq)
#         q_batch = x_query[start:end]
#         q_sq = np.sum(q_batch ** 2, axis=1, keepdims=True)
#         x_sq = np.sum(x_train ** 2, axis=1, keepdims=True).T
#         dists = q_sq + x_sq - 2 * (q_batch @ x_train.T)
#         idx = np.argpartition(dists, k, axis=1)[:, :k]
#         for i in range(end - start):
#             sorted_k = np.argsort(dists[i, idx[i]])
#             all_I[start + i] = idx[i][sorted_k]
#             all_D[start + i] = dists[i, idx[i][sorted_k]]

#     truth_I[k] = all_I
#     truth_D[k] = all_D
#     if measure_accuracy:
#         return 1.0

def search_ground_truth(k, measure_accuracy=False, indexes_dir=None, redo=True):
    global truth_I, x_query, FL2#, x_train, truth_D
    # Use FAISS flat index for brute force search to get ground truth
    # loaded = False
    if FL2 is None:
        # loaded = True
        if indexes_dir is None: raise ValueError("FL2 index not built yet")
        FL2 = faiss.read_index(os.path.join(indexes_dir, "flat.index"))
    if redo or (truth_I is None or not len(truth_I) or k not in truth_I):
        D, I = FL2.search(x_query, k)
        # if loaded and not keep_flat_index: FL2 = None  # free memory if we loaded it just for this
        truth_I[k] = I
        # truth_D[k] = D
    if measure_accuracy:
        return 1.0
    
def save_ground_truth(path):
    global truth_I#, truth_D
    assert truth_I is not None #and truth_D is not None

    # truth_I_json = {str(k): v.tolist() for k, v in truth_I.items()}
    # # truth_D_json = {str(k): v.tolist() for k, v in truth_D.items()}

    with open(path, "wb") as f:
        # json.dump(
        #     {
        #         "truth_I": truth_I_json,
        #         # "truth_D": truth_D_json
        #     },
        #     f,
        #     # indent=4 # Takes up much more disk space this way I think!
        # )
        pickle.dump(truth_I, f) # more compact and fast than json

def search(index, k, measure_accuracy=True):
    D, I = index.search(x_query, k)
    if measure_accuracy:
        global truth_I
        return batch_recall(I, truth_I[k], k)
    
def load_truth(path):
    global truth_I#, truth_D,
    with open(path, "rb") as f:
        # data = json.load(f)
        truth_I = pickle.load(f)
        # truth_I = {int(k): np.array(v) for k, v in data["truth_I"].items()}
        # # truth_D = {int(k): np.array(v) for k, v in data["truth_D"].items()}



# Index Builders
def flat_build(data):
    global FL2
    global d
    FL2 = faiss.IndexFlatL2(d)
    FL2.add(data)

def lsh_build(data, n_bits):
    global LSH
    global d
    LSH = faiss.IndexLSH(d, n_bits)
    LSH.add(data)

def pq_build(data, subquantizers, n_bits):
    global PQ
    global d
    PQ = faiss.IndexPQ(d, subquantizers, n_bits)
    PQ.train(data)
    PQ.add(data)

def ivfpq_build(data, ncentroids, code_size, n_bits):
    global IVFPQ
    global d
    coarse_quantizer = faiss.IndexFlatL2(d)
    IVFPQ = faiss.IndexIVFPQ(coarse_quantizer, d, ncentroids, code_size, n_bits)
    IVFPQ.train(data)
    IVFPQ.add(data)
    IVFPQ.nprobe = 32

def hnsw_build(data, dim, ef_construction, M, ef_search):#ef_construction=200, M=16, ef_search=100):
    global HNSW

    # OLD HNSWLIB VERSION
    # HNSW = hnswlib.Index(space='l2', dim=dim)

    # SOPHIA EDIT: FAISS HNSW BUILD
    HNSW = faiss.IndexHNSWFlat(dim, M)
    HNSW.hnsw.efConstruction = ef_construction
    HNSW.add(data)
    HNSW.hnsw.efSearch = max(int(ef_search), 1)

# def hnsw_search(k, measure_accuracy=True):
#     global HNSW, truth_I, x_query

#     # OLD HNSWLIB VERSION (COMMENTED OUT — DO NOT DELETE)
#     # labels, distances = HNSW.knn_query(x_query, k=k)

#     # SOPHIA EDIT: FAISS HNSW SEARCH
#     D, I = HNSW.search(x_query, k)
#     if measure_accuracy:
#         return batch_recall(I, truth_I[k], k)
#     return I, D

def hnsw_pq_build(data, dim, ef_construction, M, pq_m, ef_search):#ef_construction=200, M=16, pq_m=40, ef_search=100):
    global HNSWPQ
    HNSWPQ = faiss.IndexHNSWPQ(dim, pq_m, M)
    HNSWPQ.hnsw.efConstruction = ef_construction
    HNSWPQ.train(data)
    HNSWPQ.add(data)
    HNSWPQ.hnsw.efSearch = max(int(ef_search), 1)

def hnsw_sq_build(data, dim, ef_construction, M, q_type, ef_search):#ef_construction=200, M=16, q_type=faiss.ScalarQuantizer.QT_8bit, ef_search=100):
    global HNSWSQ
    HNSWSQ = faiss.IndexHNSWSQ(dim, q_type, M)
    HNSWSQ.hnsw.efConstruction = ef_construction
    HNSWSQ.train(data)
    HNSWSQ.add(data)
    HNSWSQ.hnsw.efSearch = max(int(ef_search), 1)


# Build Only (no timing)
#Sophia edit: refactored to allow building only a subset of indexes, and to save outputs in a specified subdirectory of indexes
def test_build(only=None, mem=None):
    #Build selected indexes
    #only: None or list of ["bf", "flat", "lsh", "pq", "ivfpq", "hnsw", "hsnw_pq", "hnsw_sq"]
    global d

    if only is None:
        only = ["flat", "lsh", "pq", "ivfpq", "hnsw", "hnsw_pq", "hnsw_sq"] # "bf", 

    only = set(only)

    # if "bf" in only: # No need to build; it's just a numpy array.
    #     for k in k_values:
    #         brute_force_search(k, measure_accuracy=False)  # populates truth_I and truth_D

    if "flat" in only:
        flat_build(x_train)

    for k in k_values:
        search_ground_truth(k, measure_accuracy=False)  # populates truth_I and truth_D for flat/ground truth

    if "lsh" in only:
        lsh_build(x_train, lsh_nbits)

    if "pq" in only:
        pq_build(x_train, pq_subquantizers, pq_nbits)

    if "ivfpq" in only:
        ivfpq_build(x_train, ivfpq_ncentroids, ivfpq_codesize, pq_nbits)

    if "hnsw" in only:
        # SOPHIA EDIT: ensure correct numpy data passed
        hnsw_build(x_train, d, HNSW_efconstruction, HNSW_M, HNSW_efsearch)

    if "hnsw_pq" in only:
        hnsw_pq_build(x_train, d, HNSW_efconstruction, HNSW_M, pq_subquantizers, HNSW_efsearch)

    if "hnsw_sq" in only:
        hnsw_sq_build(x_train, d, HNSW_efconstruction, HNSW_M, HNSW_SQ_qtype, HNSW_efsearch)

#    brute_force_build()
#   lsh_build(lsh_nbits)
#    pq_build(pq_subquantizers, pq_nbits)
#    ivfpq_build(ivfpq_ncentroids, ivfpq_codesize, pq_nbits)

    # SOPHIA EDIT: ensure correct numpy data passed
#    hnsw_build(x_train, d, HNSW_efconstruction, HNSW_M, HNSW_efsearch)


# MAIN

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("--num_vecs", type=int, default=None)
    parser.add_argument("--dim", type=int, default=None)
    parser.add_argument("--file", type=str, default="glove.6B.200d.txt")

    # SOPHIA EDIT: Add out_folder argument
    parser.add_argument("--out_folder", type=str, required=True,
                        help="Subdirectory inside indexes to save outputs")

    parser.add_argument("--mem_out", type=str, default=None)
    parser.add_argument("--no_save", action="store_true")
    parser.add_argument("--only", nargs="+", default=None, help = "Build only selected indexes: flat lsh pq ivfpq hnsw hnsw_pq hnsw_sq")
    args = parser.parse_args()

    filename = f"../Data/{args.file}"
    global d
    
    if filename[-4:] == ".mat": 
        # mat_data = loadmat(filename)['fea'].T
        with h5py.File(filename, 'r') as f:
            mat_data = np.array(f['fea'])

        if args.num_vecs is not None:
            mat_data = mat_data[:args.num_vecs]

        if args.dim is not None:
            mat_data = mat_data[:, :args.dim]
        # mat_data = np.ascountigousarray(mat_data, dtype=np.float32)

        d = mat_data.shape[1]

        print(f'File: "{filename}"')
        print("Dimensions:", d)

        N = len(mat_data)
        start_i = 0 # 99
        step = 100 # Makes 1/step test set
        test_i = [i for i in range(start_i, N, step)] # 1% test set
        train_i = [i for i in range(N) if (i % step != start_i)] # rest is train set
        # test_i = [i for i in range(d - 1, N, d)]
        # train_i = [i for i in range(N) if (i + d - 1) % d]
        # test_i = [i for i in range(199, N, 200)]
        # train_i = [i for i in range(N) if (i + 199) % 200]

        print("Train Size:", len(train_i))
        print("Test  Size:", len(test_i))

        x_query = mat_data[test_i]
        x_train = mat_data[train_i]


    elif filename.endswith("i8bin"): 
        with open(filename, "rb") as f:
            file_N, File_D = np.fromfile(f, dtype=np.uint32, count=2)

        bin_data = np.memmap(filename, dtype= np.int8, mode = "r", offset=8, shape=(int(file_N), int(File_D)))
        if args.num_vecs is not None:
            bin_data = bin_data[:args.num_vecs]

        if args.dim is not None:
            bin_data = bin_data[:, :args.dim]

        d = bin_data.shape[1]

        print(f'File: "{filename}"')
        print("Dimensions:", d)

        N = len(bin_data)
        start_i = 0
        step = 100

        test_i = np.arange(start_i, N, step)

        train_i = np.ones(N, dtype=bool)
        train_i[test_i] = False

        print("Train Size:", np.count_nonzero(train_i))
        print("Test  Size:", len(test_i))

        x_query = bin_data[test_i].astype(np.float32)
        x_train = bin_data[train_i].astype(np.float32)

        del bin_data 


    

    else:
        df = pd.read_csv(filename, sep=" ", quoting=3, skiprows=1, header=None)

        # vocab = df.iloc[:, 0]
        df = df.drop(0, axis=1)

        if args.num_vecs is not None:
            df = df.iloc[:args.num_vecs]

        if args.dim is not None:
            df = df.iloc[:, :args.dim]

        
        d = df.shape[1]

        print(f'File: "{filename}"')
        print("Dimensions:", d)

        N = len(df)
        start_i = 0 # 99
        step = 100 # Makes 1/step test set
        test_i = [i for i in range(start_i, N, step)] # 1% test set
        train_i = [i for i in range(N) if (i % step != start_i)] # rest is train set
        # test_i = [i for i in range(d - 1, N, d)]
        # train_i = [i for i in range(N) if (i + d - 1) % d]
        # test_i = [i for i in range(199, N, 200)]
        # train_i = [i for i in range(N) if (i + 199) % 200]

        print("Train Size:", len(train_i))
        print("Test  Size:", len(test_i))

        x_query = df.iloc[test_i].to_numpy(dtype=np.float32)
        x_train = df.iloc[train_i].to_numpy(dtype=np.float32)
        del df

    x_query = np.ascontiguousarray(x_query) # SOPHIA EDIT: convert to contiguous float32 numpy
    x_train = np.ascontiguousarray(x_train)

    dim_to_subspaces = {
        128: 32, 
        200: 40, 
        100 : 25,
        300: 30, 
        1000: 40
    }
    assert d in dim_to_subspaces

    pq_subquantizers = dim_to_subspaces[d] # Same as pq m value
    lsh_nbits = 256 # 1600
    pq_nbits = 8
    ivfpq_ncentroids = int(8 * np.sqrt(len(x_train)))
    ivfpq_codesize = pq_subquantizers * pq_nbits // 8 # code size in bytes, which is (m * n_bits) / 8
    HNSW_M = 16
    HNSW_efconstruction = 200
    HNSW_efsearch = 128
    HNSW_SQ_qtype = faiss.ScalarQuantizer.QT_8bit

    print("Building...\r", end='')

    test_build(only=args.only)

    if not args.no_save:

        # SOPHIA EDIT: Save inside indexes/<out_folder>/
        base_dir = os.path.join("indexes", args.out_folder)
        os.makedirs(base_dir, exist_ok=True)

        #Sophia EDIT: allow saving only a subset of indexes, based on --only argument
        build = set(args.only) if args.only is not None else {"flat", "lsh", "pq", "ivfpq", "hnsw", "hnsw_pq", "hnsw_sq"} #"bf", 

        # Save the ground truth for recall calculations.
        # if "flat" in build: # was "bf"
        save_ground_truth(os.path.join(base_dir, TRUTH_FILE_NAME))
        # if "bf" in build:
        #     assert truth_I is not None and truth_D is not None
        #     with open(os.path.join(base_dir, "truth_I,D.json"), "w") as f:
        #         json.dump({"truth_I": truth_I, "truth_D": truth_D}, f, indent=4)

        if "flat" in build and FL2 is not None:
            faiss.write_index(FL2, os.path.join(base_dir, "flat.index"))
    
        if "lsh" in build and LSH is not None:
            faiss.write_index(LSH, os.path.join(base_dir, "lsh.index"))
        
        if "pq" in build and PQ is not None:
            faiss.write_index(PQ, os.path.join(base_dir, "pq.index"))

        if "ivfpq" in build and IVFPQ is not None:
            faiss.write_index(IVFPQ, os.path.join(base_dir, "ivfpq.index"))

        if "hnsw" in build and HNSW is not None:
            faiss.write_index(HNSW, os.path.join(base_dir, "hnsw.index"))

        if "hnsw_pq" in build and HNSWPQ is not None:
            faiss.write_index(HNSWPQ, os.path.join(base_dir, "hnsw_pq.index"))

        if "hnsw_sq" in build and HNSWSQ is not None:
            faiss.write_index(HNSWSQ, os.path.join(base_dir, "hnsw_sq.index"))

        # faiss.write_index(FL2,   os.path.join(base_dir, "flat.index"))
        # faiss.write_index(LSH,   os.path.join(base_dir, "lsh.index"))
        # faiss.write_index(PQ,    os.path.join(base_dir, "pq.index"))
        # faiss.write_index(IVFPQ, os.path.join(base_dir, "ivfpq.index"))
        # faiss.write_index(HNSW,  os.path.join(base_dir, "hnsw.index"))

        np.save(os.path.join(base_dir, "x_query.npy"), x_query)
        np.save(os.path.join(base_dir, "x_train.npy"), x_train)

        with open(os.path.join(base_dir, "meta.json"), "w") as f:
            json.dump({
                "source_file": args.file,
                "d": int(d),
                "num_vecs": int(N),

                "lsh_nbits": int(lsh_nbits),
                "pq_nbits": int(pq_nbits),
                "pq_subquantizers": int(pq_subquantizers),
                "ivfpq_codesize": int(ivfpq_codesize),
                "ivfpq_ncentroids": int(ivfpq_ncentroids),
                "HNSW_M": int(HNSW_M),
                "HNSW_efconstruction": int(HNSW_efconstruction),
                "HNSW_efsearch": int(HNSW_efsearch),
                "HNSW_SQ_qtype": int(HNSW_SQ_qtype),
            }, f, indent=4)

    print("Build complete.")
