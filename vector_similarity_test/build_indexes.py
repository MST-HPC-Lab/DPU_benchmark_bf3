import pandas as pd
import numpy as np
import faiss
from timeit import repeat
import hnswlib


# Recall Measurement functions
truth_D, truth_I = None, None
def recall_at_k(test, truth, k):
    assert len(test) == k
    assert len(truth) == k
    return len(set(truth).intersection(test)) / k
def batch_recall(test_I, truth_I, k):
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

truth_I = None
truth_D = None
def brute_force_search(k, measure_accuracy=False):
    global FL2, truth_D, truth_I
    truth_D, truth_I = FL2.search(x_query, k) # D: Distances, I: Indices
    if measure_accuracy:
        return 1.0 # Accuracy is perfect because we're treating this as ground truth

# Search function is used for all other indexes
def search(index, k, measure_accuracy=True):
    # Must run brute_force_search first, for results to compare accuracy against
    D, I = index.search(x_query, k)
    if measure_accuracy:
        global truth_I;
        return batch_recall(I, truth_I, k)
#hnsw search
def hnsw_search(k, measure_accuracy=True):
    global HNSW, truth_I, x_query
    labels, distances = HNSW.knn_query(x_query, k=k)

    if measure_accuracy:
        return batch_recall(labels, truth_I, k)
    
    return labels, distances

#------------------------------------------

# LSH Index
LSH = None
def lsh_build(n_bits):
    global LSH;
    LSH = faiss.IndexLSH (d, n_bits)
    # LSH.train (x_train) # LSH Technically doesn't require training
    LSH.add (x_train) # NOTE: Can add other indexed data than what it's trained on

# PQ Index
PQ = None
def pq_build(subquantizers, n_bits):
    global PQ;
    PQ = faiss.IndexPQ(d, subquantizers, n_bits)
    PQ.train(x_train)
    PQ.add(x_train)

# IVF PQ Index
IVFPQ = None
def ivfpq_build(ncentroids, code_size, n_bits):
    global IVFPQ;
    # ncentroids = Number of Voronoi cells
    # code_size = Number of sub-vectors (dimension partitions) for product quantization
    coarse_quantizer = faiss.IndexFlatL2(d)
    IVFPQ = faiss.IndexIVFPQ(coarse_quantizer, d, ncentroids, code_size, n_bits)
    IVFPQ.train(x_train)
    IVFPQ.add(x_train)
    IVFPQ.nprobe = 5

# HNSW Index
HNSW = None
def hnsw_build(data, dim, ef_construction=200, M=16, ef_search=100):
    global HNSW
    HNSW = hnswlib.Index(space='l2', dim=dim)
    HNSW.init_index(max_elements=data.shape[0], ef_construction=ef_construction, M=M)
    HNSW.add_items(data)
    HNSW.set_ef(ef_search)

#------------------------------------------

# To time both building and searching
def test_suite(k=1, r=1):
    # k = recall set size
    # r = number of repeats to get more accurate average timings
    print(f"Measuring: {k}-recall@{k}")
    print(f"Repeats: {r} (All times averaged across)")
    
    # Brute Force Ground Truth ----------------
    print("---------------------- Brute Force ----------------------")
    bf_btime = np.mean(repeat(lambda: brute_force_build(),  number=1, repeat=r))#, globals=globals()))
    bf_stime = np.mean(repeat(lambda: brute_force_search(k), number=1, repeat=r))#, globals=globals()))

    print("Brute Force Accuracy: 1.0")
    print("Brute Force Build  Time:", bf_btime)
    print("Brute Force Search Time:", bf_stime)


    # LSH -------------------------------------
    print("---------------------- LSH ----------------------")
    global LSH, lsh_nbits;
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
    global PQ, pq_nbits, pq_subquantizers;
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
    global IVFPQ, ivfpq_ncentroids, ivfpq_codesize;
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


# To build without timing it
def test_build():
    brute_force_build()
    lsh_build(lsh_nbits)
    pq_build(pq_subquantizers, pq_nbits)
    ivfpq_build(ivfpq_ncentroids, ivfpq_codesize, pq_nbits)
    hnsw_build(x_train.to_numpy(), d, HNSW_M, HNSW_efconstruction, HNSW_efsearch)

# To time the search only
def test_search(k=1, r=1, verbose=True):
    # k = recall set size
    # r = number of repeats to get more accurate average timings
    if verbose: print(f"Measuring: {k}-recall@{k}")
    if verbose: print(f"Repeats: {r} (All times averaged across)")
    
    # Brute Force Ground Truth ----------------
    if verbose: print("---------------------- Brute Force ----------------------")
    bf_stime = np.mean(repeat(lambda: brute_force_search(k), number=1, repeat=r))#, globals=globals()))
    if verbose: print("Brute Force avg Search Time:", bf_stime)


    # LSH -------------------------------------
    if verbose: print("---------------------- LSH ----------------------")
    global LSH, lsh_nbits;
    if verbose: print("LSH n_bits:", lsh_nbits)
    lsh_result = search(LSH, k)
    lsh_stime = np.mean(repeat(lambda: search(LSH, k, measure_accuracy=False),  number=1, repeat=r))

    if verbose: print("LSH recall:", lsh_result)
    if verbose: print("LSH avg search time:", lsh_stime)
    if verbose: print(f"LSH search Speedup compared to brute force: {bf_stime/lsh_stime}")


    # PQ -------------------------------------
    if verbose: print("---------------------- PQ ----------------------")
    global PQ, pq_nbits, pq_subquantizers;
    if verbose: print("PQ n_bits:", pq_nbits)
    if verbose: print("PQ Subquantizers:", pq_subquantizers)
    pq_result = search(PQ, k)
    pq_stime = np.mean(repeat(lambda: search(PQ, k, measure_accuracy=False),  number=1, repeat=r))

    if verbose: print("PQ recall:", pq_result)
    if verbose: print("PQ search time:", pq_stime)
    if verbose: print(f"PQ search Speedup: {bf_stime/pq_stime}")


    # IVF PQ -------------------------------------
    if verbose: print("---------------------- IVF PQ ----------------------")
    global IVFPQ, ivfpq_ncentroids, ivfpq_codesize;
    if verbose: print("IVF PQ n_bits:", pq_nbits)
    if verbose: print("IVF PQ ncentroids:", ivfpq_ncentroids)
    if verbose: print("IVF PQ codesize:", ivfpq_codesize)
    ivfpq_result = search(IVFPQ, k)
    ivfpq_stime = np.mean(repeat(lambda: search(IVFPQ, k, measure_accuracy=False),  number=1, repeat=r))

    if verbose: print("IVFPQ recall:", ivfpq_result)
    if verbose: print("IVFPQ search time:", ivfpq_stime)
    if verbose: print(f"IVFPQ search Speedup: {bf_stime/ivfpq_stime}")

     #HNSW
    if verbose: print("---------------------- HNSW ----------------------")
    hnsw_result = hnsw_search(k)
    hnsw_stime = np.mean(repeat(lambda: hnsw_search(k, measure_accuracy=False), number=1, repeat=r))

    if verbose: print("HNSW recall:", hnsw_result)
    if verbose: print("HNSW avg search time:", hnsw_stime)
    if verbose: print(f"HNSW search Speedup: {bf_stime/hnsw_stime}")

    return [bf_stime, lsh_result, lsh_stime, pq_result, pq_stime, ivfpq_result, ivfpq_stime, hnsw_result, hnsw_stime]


#===============================================================================


if __name__ == "__main__":
    # Load Data File
    # colnames = ["VOCAB"] + [str(i) for i in range(200)]
    filename = "../Data/cc.en.300.vec" # ../Data/glove.6b.200d.txt
    df = pd.read_csv(filename, sep=" ", quoting=3, skiprows=1,  header=None)
    # df = pd.read_csv(filename, sep=" ", quoting=3, skiprows=1, header=None) #added skiprow so that it skips the header row in fast text file

    # Split into vocab column and data
    vocab = df.iloc[:,0]
    df = df.drop(0, axis=1)
    d = len(df.iloc[0])
    print(f'File: "{filename}"')
    print("Dimensions:", d)

   # Testing vs. Training Split
   #test_i = [i for i in range(399,400000,400)]
    test_i = [i for i in range(199,2000000,200)]# len is ~2000, len of fastext vocab 2 million
   # train_i = [i for i in range(400000) if (i+399)%400]
    train_i = [i for i in range(2000000) if (i+199)%200]# len is ~398000
    x_query = df.iloc[test_i]
    x_train = df.iloc[train_i]
    print("Train Size:", len(train_i))
    print("Test  Size:", len(test_i))

    # # Presets based on 200 dimensions and best results with k=10
    # lsh_nbits = 8*d
    # pq_nbits = 12
    # pq_subquantizers = 40
    # ivfpq_ncentroids = 8
    # ivfpq_codesize = 40

    # Presets based on 200 dimensions and more efficient results, fastest that is still perfect with k=1, though it ends up being poor with k=10
    # lsh_nbits = 1600 # 2 * d
    # pq_nbits = 8
    # pq_subquantizers = 40
    # ivfpq_ncentroids = 5
    # ivfpq_codesize = 40
    # HNSW_M = 32
    # HNSW_efconstruction = 256
    # HNSW_efsearch = 128

    # Presets based on 300 dimensions 
    lsh_nbits = 1600 # 2 * d
    pq_nbits = 8
    pq_subquantizers = 30
    ivfpq_ncentroids = 5
    ivfpq_codesize = 30
    HNSW_M = 32
    HNSW_efconstruction = 256
    HNSW_efsearch = 128


    # Run Full Test
    # test_suite(k=1, r=1)

    # Run search timing test with various k
    print("Building...\r", end='')
    test_build()
    results = []
    for k in [1, 2, 3, 5, 7, 10, 25, 50, 75, 100]:
        print(f"Searching with k={k}...\r", end='')
        results.append(test_search(k=k, r=3, verbose=False))
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

"""
from datasketch import MinHash
m = MinHash()
for d in df:
    m.update(d.encode('utf8'))
print("Estimated Jaccard for m and m2:", m.jaccard(m2))
# More: https://ekzhu.com/datasketch/minhash.html
"""
