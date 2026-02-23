import pandas as pd
import numpy as np
import faiss
from timeit import repeat

# import hnswlib  # SOPHIA EDIT: removed hnswlib, switching fully to FAISS HNSW


# Load Data File
df = pd.read_csv("../Data/glove.6B.200d.txt", sep=" ", quoting=3, header=None)

d = 200
k = 100

# Split into vocab column and data
vocab = df.iloc[:, 0]
df = df.drop(0, axis=1)

# Testing vs. Training Split
test_i = [i for i in range(399, 400000, 400)]
train_i = [i for i in range(400000) if (i + 399) % 400]

x_query = df.iloc[test_i]
x_train = df.iloc[train_i]

x_train_np = np.ascontiguousarray(x_train.to_numpy(dtype=np.float32))
x_query_np = np.ascontiguousarray(x_query.to_numpy(dtype=np.float32))

x_train = x_train_np
x_query = x_query_np


# Recall Measurement Functions
truth_D, truth_I = None, None

def k_recall_at_k(test, truth, k=k):
    return len(set(truth).intersection(test)) / k

def batch_recall(test_I, truth_I, k=k):
    results = []
    for i, row in enumerate(test_I):
        results.append(k_recall_at_k(row, truth_I[i], k))
    return sum(results) / len(results)


# Flat Index (Brute Force Ground Truth)
IFL2 = None

def brute_force_build():
    global IFL2
    IFL2 = faiss.IndexFlatL2(d)
    IFL2.add(x_train)

def brute_force_search(k=k, measure_accuracy=True):
    global truth_D, truth_I
    truth_D, truth_I = IFL2.search(x_query, k)
    if measure_accuracy:
        return 1.0


# Generic search function
def search(index, k=k, measure_accuracy=True):
    D, I = index.search(x_query, k)
    if measure_accuracy:
        global truth_I
        return batch_recall(I, truth_I)


# HNSW Index
HNSW = None

def hnsw_build(data, dim, ef_construction=200, M=16, ef_search=100):
    global HNSW

    # OLD HNSWLIB VERSION)
    # arr = x_train_np
    # HNSW = hnswlib.Index(space='l2', dim=dim)
    # HNSW.init_index(max_elements=arr.shape[0], ef_construction=ef_construction, M=M)
    # HNSW.add_items(arr)
    # HNSW.set_ef(ef_search)

    # SOPHIA EDIT: REPLACED WITH FAISS HNSW
    HNSW = faiss.IndexHNSWFlat(dim, M)
    HNSW.hnsw.efConstruction = ef_construction
    HNSW.add(x_train_np)
    HNSW.hnsw.efSearch = max(ef_search, k)


def hnsw_search(k=k, measure_accuracy=True):
    global HNSW, truth_I

    # OLD HNSWLIB VERSION 
    # labels, distances = HNSW.knn_query(x_query_np, k=k)
    # if measure_accuracy:
    #     return batch_recall(labels, truth_I, k)
    # return labels, distances

    # SOPHIA EDIT: FAISS SEARCH
    D, I = HNSW.search(x_query_np, k)

    if measure_accuracy:
        return batch_recall(I, truth_I, k)

    return I, D

# HNSW Parameter Sweep
hnsw_M_list = [8, 16, 32]
hnsw_efc_list = [100, 200, 400]
hnsw_efs_list = [k, max(2*k, 50), 100, 200, 400]


def test_suite(r=3):

    recall_str = f"{k}-recall@{k}"
    print(f"(All times averaged over {r} repeats)")

    # Brute Force Ground Truth
    print("Brute Force")

    bf_btime = np.mean(repeat(lambda: brute_force_build(), number=1, repeat=r))
    bf_stime = np.mean(repeat(lambda: brute_force_search(), number=1, repeat=r))

    print("Brute Force Accuracy: 1.0")
    print("Brute Force Build Time:", bf_btime)
    print("Brute Force Search Time:", bf_stime)

    # HNSW (FAISS)
    print("HNSW (FAISS)")

    for M in hnsw_M_list:
        print(f"\n HNSW M: {M}")

        for efc in hnsw_efc_list:

            bt = np.mean(repeat(
                lambda: hnsw_build(x_train, d, ef_construction=efc, M=M, ef_search=max(k, 100)),
                number=1, repeat=r
            ))

            print(f"ef_construction: {efc}, build_time: {bt:.4f}s")

            hnsw_build(x_train, d, ef_construction=efc, M=M, ef_search=max(k, 100))

            for efs in hnsw_efs_list:

                HNSW.hnsw.efSearch = max(efs, k)

                rc = hnsw_search(k)

                st = np.mean(repeat(
                    lambda: hnsw_search(k, measure_accuracy=False),
                    number=1, repeat=r
                ))

                print(f"ef_search: {efs}, recall: {rc:.4f}, search_time: {st:.6f}s")


if __name__ == "__main__":
    test_suite()