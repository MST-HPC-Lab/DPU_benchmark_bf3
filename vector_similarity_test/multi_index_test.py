import pandas as pd
import numpy as np
import faiss
from timeit import repeat

# Load Data File
# df = pd.read_csv("../Data/glove.6B.200d.txt", sep=" ", quoting=3, header=None)
df = pd.read_csv("../Data/glove.6B.200d.txt", sep=" ", quoting=3, skiprows=1, header=None)

d = 200
k = 100

# Split into vocab column and data
vocab = df.iloc[:, 0]
df = df.drop(0, axis=1)

# Fixed Split
N = len(df)
test_i = [i for i in range(199, N, 200)]
train_i = [i for i in range(N) if (i + 199) % 200]

x_query = df.iloc[test_i]
x_train = df.iloc[train_i]

x_train = np.ascontiguousarray(x_train.to_numpy(dtype=np.float32))
x_query = np.ascontiguousarray(x_query.to_numpy(dtype=np.float32))

# Recall
truth_D, truth_I = None, None

def k_recall_at_k(test, truth, k=k):
    return len(set(truth).intersection(test)) / k

def batch_recall(test_I, truth_I, k=k):
    return np.mean([k_recall_at_k(row, truth_I[i], k) for i, row in enumerate(test_I)])

# Timing
def avg_time(fn, reps=3):
    fn()
    return np.mean(repeat(fn, number=1, repeat=reps))

# Flat
FLAT = None

def brute_force_build():
    global FLAT
    FLAT = faiss.IndexFlatL2(d)
    FLAT.add(x_train)

def brute_force_search(k=k):
    global truth_D, truth_I
    truth_D, truth_I = FLAT.search(x_query, k)

# HNSW
HNSW = None

def hnsw_build(M=16, efc=200, efs=200):
    global HNSW
    HNSW = faiss.IndexHNSWFlat(d, M)
    HNSW.hnsw.efConstruction = efc
    HNSW.add(x_train)
    HNSW.hnsw.efSearch = max(int(efs), 1)

def hnsw_search(k=k, measure_accuracy=True):
    D, I = HNSW.search(x_query, k)
    if measure_accuracy:
        return batch_recall(I, truth_I, k)
    return I, D

# LSH 
LSH = None

def lsh_build(nbits=256):
    global LSH
    LSH = faiss.IndexLSH(d, nbits)
    LSH.add(x_train)

def lsh_search(k=k, measure_accuracy=True):
    D, I = LSH.search(x_query, k)
    if measure_accuracy:
        return batch_recall(I, truth_I, k)
    return I, D

# PQ 
PQ = None

def pq_build(m=8, nbits=8):
    global PQ
    PQ = faiss.IndexPQ(d, m, nbits)
    PQ.train(x_train)
    PQ.add(x_train)

def pq_search(k=k, measure_accuracy=True):
    D, I = PQ.search(x_query, k)
    if measure_accuracy:
        return batch_recall(I, truth_I, k)
    return I, D

# IVFPQ
IVFPQ = None

def ivfpq_build(nlist=100, m=8, nbits=8):
    global IVFPQ
    quantizer = faiss.IndexFlatL2(d)
    IVFPQ = faiss.IndexIVFPQ(quantizer, d, nlist, m, nbits)
    IVFPQ.train(x_train)
    IVFPQ.add(x_train)

def ivfpq_search(k=k, measure_accuracy=True):
    D, I = IVFPQ.search(x_query, k)
    if measure_accuracy:
        return batch_recall(I, truth_I, k)
    return I, D

# TEST
def test_suite(r=3):

    print(f"(All times averaged over {r} repeats)")

    # Ground Truth
    print("\nBrute Force")
    bf_build = avg_time(brute_force_build, r)
    brute_force_build()
    bf_search = avg_time(lambda: brute_force_search(k), r)

    print("Build Time:", bf_build)
    print("Search Time:", bf_search)

    # HNSW 
    print("\nHNSW")
    for M in [8, 16, 32]:
        for efc in [100, 200]:
            for efs in [100, 200]:

                bt = avg_time(lambda: hnsw_build(M, efc, efs), r)
                hnsw_build(M, efc, efs)

                brute_force_search(k)

                rc = hnsw_search(k)
                st = avg_time(lambda: hnsw_search(k, False), r)

                print(f"M={M}, efc={efc}, efs={efs} | recall={rc:.3f}, time={st:.4f}")

    # LSH 
    print("\nLSH")
    for nbits in [128, 256, 512]:

        bt = avg_time(lambda: lsh_build(nbits), r)
        lsh_build(nbits)

        brute_force_search(k)

        rc = lsh_search(k)
        st = avg_time(lambda: lsh_search(k, False), r)

        print(f"nbits={nbits} | recall={rc:.3f}, time={st:.4f}")

    # PQ
    print("\nPQ")
    for m in [4, 5, 10, 20]:
    
        bt = avg_time(lambda: pq_build(m), r)
        pq_build(m)

        brute_force_search(k)

        rc = pq_search(k)
        st = avg_time(lambda: pq_search(k, False), r)

        print(f"m={m} | recall={rc:.3f}, time={st:.4f}")

    # IVFPQ 
    print("\nIVFPQ")
    for nlist in [50, 100, 200]:

        bt = avg_time(lambda: ivfpq_build(nlist), r)
        ivfpq_build(nlist)

        brute_force_search(k)

        rc = ivfpq_search(k)
        st = avg_time(lambda: ivfpq_search(k, False), r)

        print(f"nlist={nlist} | recall={rc:.3f}, time={st:.4f}")

if __name__ == "__main__":
    test_suite()