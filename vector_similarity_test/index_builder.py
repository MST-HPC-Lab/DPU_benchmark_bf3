# index_builder.py

import argparse
import pandas as pd
import numpy as np
import faiss
from timeit import repeat
# import hnswlib  # SOPHIA EDIT: removed hnswlib, switching to FAISS HNSW
import os, json

from mem_utils import MemoryMonitor
os.makedirs("results", exist_ok=True)

# Globals used across functions
truth_D, truth_I = None, None
FL2 = None
LSH = None
PQ = None
IVFPQ = None
HNSW = None


# Recall helpers

def recall_at_k(test, truth, k):
    assert len(test) == k
    assert len(truth) == k
    return len(set(truth).intersection(test)) / k

def batch_recall(test_I, truth_I, k):
    results = []
    for i, row in enumerate(test_I):
        results.append(recall_at_k(row, truth_I[i], k))
    return sum(results) / len(results)


# Flat index

def brute_force_build():
    global FL2
    FL2 = faiss.IndexFlatL2(d)
    FL2.add(x_train)

def brute_force_search(k, measure_accuracy=False):
    global FL2, truth_D, truth_I
    truth_D, truth_I = FL2.search(x_query, k)
    if measure_accuracy:
        return 1.0


def search(index, k, measure_accuracy=True):
    D, I = index.search(x_query, k)
    if measure_accuracy:
        global truth_I
        return batch_recall(I, truth_I, k)


# HNSW

def hnsw_search(k, measure_accuracy=True):
    global HNSW, truth_I, x_query

    # OLD HNSWLIB VERSION (COMMENTED OUT — DO NOT DELETE)
    # labels, distances = HNSW.knn_query(x_query, k=k)

    # SOPHIA EDIT: FAISS HNSW SEARCH
    D, I = HNSW.search(x_query, k)
    if measure_accuracy:
        return batch_recall(I, truth_I, k)
    return I, D


def hnsw_build(data, dim, ef_construction=200, M=16, ef_search=100):
    global HNSW

    # OLD HNSWLIB VERSION
    # HNSW = hnswlib.Index(space='l2', dim=dim)

    # SOPHIA EDIT: FAISS HNSW BUILD
    HNSW = faiss.IndexHNSWFlat(dim, M)
    HNSW.hnsw.efConstruction = ef_construction
    HNSW.add(data)
    HNSW.hnsw.efSearch = max(int(ef_search), 1)


# Other Index Builders

def lsh_build(n_bits):
    global LSH
    LSH = faiss.IndexLSH(d, n_bits)
    LSH.add(x_train)

def pq_build(subquantizers, n_bits):
    global PQ
    PQ = faiss.IndexPQ(d, subquantizers, n_bits)
    PQ.train(x_train)
    PQ.add(x_train)

def ivfpq_build(ncentroids, code_size, n_bits):
    global IVFPQ
    coarse_quantizer = faiss.IndexFlatL2(d)
    IVFPQ = faiss.IndexIVFPQ(coarse_quantizer, d, ncentroids, code_size, n_bits)
    IVFPQ.train(x_train)
    IVFPQ.add(x_train)
    IVFPQ.nprobe = 5


# Build Only (no timing)

def test_build(mem=None):

    brute_force_build()
    lsh_build(lsh_nbits)
    pq_build(pq_subquantizers, pq_nbits)
    ivfpq_build(ivfpq_ncentroids, ivfpq_codesize, pq_nbits)

    # SOPHIA EDIT: ensure correct numpy data passed
    hnsw_build(x_train, d, HNSW_efconstruction, HNSW_M, HNSW_efsearch)


# MAIN

if __name__ == "__main__":

    parser = argparse.ArgumentParser()
    parser.add_argument("--num_vecs", type=int, default=None)
    parser.add_argument("--dim", type=int, default=None)
    parser.add_argument("--file", type=str, default="cc.en.300.vec")

    # SOPHIA EDIT: Add out_folder argument
    parser.add_argument("--out_folder", type=str, required=True,
                        help="Subdirectory inside indices/ to save outputs")

    parser.add_argument("--mem_out", type=str, default=None)
    parser.add_argument("--no_save", action="store_true")
    args = parser.parse_args()

    filename = f"../Data/{args.file}"
    df = pd.read_csv(filename, sep=" ", quoting=3, skiprows=1, header=None)

    vocab = df.iloc[:, 0]
    df = df.drop(0, axis=1)

    if args.num_vecs is not None:
        df = df.iloc[:args.num_vecs]

    if args.dim is not None:
        df = df.iloc[:, :args.dim]

    d = df.shape[1]

    dim_to_subspaces = {128: 32, 200: 40, 300: 30, 1000: 40}

    if d in dim_to_subspaces:
        pq_subquantizers = dim_to_subspaces[d]
        ivfpq_codesize = dim_to_subspaces[d]

    print(f'File: "{filename}"')
    print("Dimensions:", d)

    N = len(df)
    test_i = [i for i in range(199, N, 200)]
    train_i = [i for i in range(N) if (i + 199) % 200]

    x_query = df.iloc[test_i]
    x_train = df.iloc[train_i]

    print("Train Size:", len(train_i))
    print("Test  Size:", len(test_i))

    # SOPHIA EDIT: convert to contiguous float32 numpy
    x_query = np.ascontiguousarray(x_query.to_numpy(dtype=np.float32))
    x_train = np.ascontiguousarray(x_train.to_numpy(dtype=np.float32))

    lsh_nbits = 1600
    pq_nbits = 8
    ivfpq_ncentroids = 5
    HNSW_M = 32
    HNSW_efconstruction = 256
    HNSW_efsearch = 128

    print("Building...\r", end='')

    test_build()

    if not args.no_save:

        # SOPHIA EDIT: Save inside indices/<out_folder>/
        base_dir = os.path.join("indices", args.out_folder)
        os.makedirs(base_dir, exist_ok=True)

        faiss.write_index(FL2,   os.path.join(base_dir, "flat.index"))
        faiss.write_index(LSH,   os.path.join(base_dir, "lsh.index"))
        faiss.write_index(PQ,    os.path.join(base_dir, "pq.index"))
        faiss.write_index(IVFPQ, os.path.join(base_dir, "ivfpq.index"))
        faiss.write_index(HNSW,  os.path.join(base_dir, "hnsw.index"))

        np.save(os.path.join(base_dir, "x_query.npy"), x_query)
        np.save(os.path.join(base_dir, "x_train.npy"), x_train)

        with open(os.path.join(base_dir, "meta.json"), "w") as f:
            json.dump({
                "d": int(d),
                "lsh_nbits": int(lsh_nbits),
                "pq_nbits": int(pq_nbits),
                "pq_subquantizers": int(pq_subquantizers),
                "ivfpq_codesize": int(ivfpq_codesize),
                "ivfpq_ncentroids": int(ivfpq_ncentroids),
                "HNSW_M": int(HNSW_M),
                "HNSW_efconstruction": int(HNSW_efconstruction),
                "HNSW_efsearch": int(HNSW_efsearch),
                "source_file": args.file,
                "num_vecs": int(N),
            }, f)

    print("Build complete.")