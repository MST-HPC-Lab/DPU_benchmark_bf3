import pandas as pd
import numpy as np
import faiss
from timeit import repeat


# Load Data File
# colnames = ["VOCAB"] + [str(i) for i in range(200)]
df = pd.read_csv("../Data/glove.6B.200d.txt", sep=" ", quoting=3, header=None)
d = 200
k = 10

# Split into vocab column and data
vocab = df.iloc[:,0]
df = df.drop(0, axis=1)

# Testing vs. Training Split
test_i = [i for i in range(199,400000,200)] # len is ~2000
train_i = [i for i in range(400000) if (i+199)%200]
x_query = df.iloc[test_i]
x_train = df.iloc[train_i]

# Recall Measurement functions
truth_D, truth_I = None, None
def recall_at_k(test, truth, k=k):
    assert len(test) == k
    assert len(truth) == k
    return len(set(truth).intersection(test)) / k
def batch_recall(test_I, truth_I, k=k):
    results = []
    for i, row in enumerate(test_I):
        results.append(recall_at_k(row, truth_I[i], k))
        # print(results[-1])
    # return np.mean(results)
    return sum(results) / len(results)

#------------------------------------------

# Flat Index (Brute Force)
FL2 = None
def brute_force_build():
    global FL2;
    FL2 = faiss.IndexFlatL2(d)
    FL2.add(x_train)

def brute_force_search(k=k, measure_accuracy=False):
    global truth_D, truth_I
    truth_D, truth_I = FL2.search(x_query, k) # D: Distances, I: Indices
    if measure_accuracy:
        return 1.0 # Accuracy is perfect because we're treating this as ground truth

# Search function is used for all other indexes
def search(index, k=k, measure_accuracy=True):
    # Must run brute_force_search first, for results to compare accuracy against
    D, I = index.search(x_query, k)
    if measure_accuracy:
        global truth_I;
        return batch_recall(I, truth_I)

#------------------------------------------

lsh_nbits = 8*d
pq_nbits = 12
pq_subquantizers = 40
ivfpq_ncentroids = 8
ivfpq_codesize = 40

# LSH Index
LSH = None
def lsh_build(n_bits):
    LSH = faiss.IndexLSH (d, n_bits)
    # LSH.train (x_train) # LSH Technically doesn't require training
    LSH.add (x_train) # NOTE: Can add other indexed data than what it's trained on

# PQ Index
PQ = None
def pq_build(subquantizers, n_bits):
    PQ = faiss.IndexPQ(d, subquantizers, n_bits)
    PQ.train(x_train)
    PQ.add(x_train)

# IVF PQ Index
IVFPQ = None
def ivfpq_build(ncentroids, code_size, n_bits):
    # ncentroids = Number of Voronoi cells
    # code_size = Number of sub-vectors (dimension partitions) for product quantization
    coarse_quantizer = faiss.IndexFlatL2(d)
    IVFPQ = faiss.IndexIVFPQ(coarse_quantizer, d, ncentroids, code_size, n_bits)
    IVFPQ.train(x_train)
    IVFPQ.add(x_train)
    IVFPQ.nprobe = 5

#------------------------------------------

def test_suite(r=1): # r = number of repeats to get more accurate average timings
    recall_str = f"{k}-recall@{k}"
    print(f"(All times averaged out of {r} repeats)")
    
    # Brute Force Ground Truth ----------------
    print("---------------------- Brute Force ----------------------")
    bf_btime = np.mean(repeat(lambda: brute_force_build(),  number=1, repeat=r))#, globals=globals()))
    bf_stime = np.mean(repeat(lambda: brute_force_search(), number=1, repeat=r))#, globals=globals()))

    print("Brute Force Accuracy: 1.0")
    print("Brute Force Build  Time:", bf_btime)
    print("Brute Force Search Time:", bf_stime)


    # LSH -------------------------------------
    print("---------------------- LSH ----------------------")
    global lsh_nbits;
    print("LSH n_bits:", lsh_nbits)
    lsh_btime = np.mean(repeat(lambda: lsh_build(lsh_nbits),  number=1, repeat=r))
    lsh_result = search(LSH, k)
    lsh_stime = np.mean(repeat(lambda: search(LSH, k, measure_accuracy=False),  number=1, repeat=r))

    print("LSH recall:", lsh_result)
    print(f"LSH build time: {lsh_btime}")
    print("LSH search time:", lsh_stime)
    print(f"LSH search Speedup compared to brute force: {bf_stime/lsh_stime}")
    print(f"LSH build  Speedup compared to brute force: {bf_btime/lsh_btime}")


    # PQ -------------------------------------
    print("---------------------- PQ ----------------------")
    global pq_nbits, pq_subquantizers;
    print("PQ n_bits:", pq_nbits)
    print("PQ Subquantizers:", pq_subquantizers)
    pq_btime = np.mean(repeat(lambda: pq_build(pq_subquantizers, pq_nbits),  number=1, repeat=r))
    pq_result = search(PQ, k)
    pq_stime = np.mean(repeat(lambda: search(PQ, k, measure_accuracy=False),  number=1, repeat=r))

    print("PQ recall:", pq_result)
    print(f"PQ build time: {pq_btime}")
    print("PQ search time:", pq_stime)
    print(f"PQ search Speedup: {bf_stime/pq_stime}")
    print(f"PQ build  Speedup: {bf_btime/pq_btime}")


    # IVF PQ -------------------------------------
    print("---------------------- IVF PQ ----------------------")
    global ivfpq_ncentroids, ivfpq_codesize;
    print("IVF PQ n_bits:", pq_nbits)
    print("IVF PQ ncentroids:", ivfpq_ncentroids)
    print("IVF PQ codesize:", ivfpq_codesize)
    ivfpq_btime = np.mean(repeat(lambda: ivfpq_build(ivfpq_ncentroids, ivfpq_codesize, pq_nbits),  number=1, repeat=r))
    ivfpq_result = search(IVFPQ, k)
    ivfpq_stime = np.mean(repeat(lambda: search(IVFPQ, k, measure_accuracy=False),  number=1, repeat=r))

    print("IVFPQ recall:", ivfpq_result)
    print(f"IVFPQ build time: {ivfpq_btime}")
    print("IVFPQ search time:", ivfpq_stime)
    print(f"IVFPQ search Speedup: {bf_stime/ivfpq_stime}")
    print(f"IVFPQ build  Speedup: {bf_btime/ivfpq_btime}")


    # All -------------------------------------
    print("---------------------- ALL ----------------------")
    indexes = ["LSH", "PQ", "IVFPQ"]
    best_times = [lsh_stime, pq_stime, ivfpq_stime]
    best_timeb = [lsh_btime, pq_btime, ivfpq_btime]
    best_accuracy = [lsh_result, pq_result, ivfpq_result]
    best_i = np.argmax(best_accuracy)
    fastb_i = np.argmin(best_timeb)
    fasts_i = np.argmin(best_times)

    print("Most Accurate:", indexes[best_i])
    print("Fastest Build:", indexes[fastb_i])
    print("Fastest Search:", indexes[fasts_i])



if __name__ == "__main__":
    test_suite(r=1)

"""
from datasketch import MinHash
m = MinHash()
for d in df:
    m.update(d.encode('utf8'))
print("Estimated Jaccard for m and m2:", m.jaccard(m2))
# More: https://ekzhu.com/datasketch/minhash.html
"""