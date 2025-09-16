import pandas as pd
import numpy as np
import faiss
from timeit import repeat
import hnswlib

# Load Data File
# colnames = ["VOCAB"] + [str(i) for i in range(200)]
df = pd.read_csv("../Data/glove.6B.200d.txt", sep=" ", quoting=3, header=None)
d = 200
k = 10

# Split into vocab column and data
vocab = df.iloc[:,0]
df = df.drop(0, axis=1)

# Testing vs. Training Split
test_i = [i for i in range(399,400000,400)]
train_i = [i for i in range(400000) if (i+399)%400]
x_query = df.iloc[test_i]
x_train = df.iloc[train_i]

x_train_np = np.ascontiguousarray(x_train.to_numpy(dtype=np.float32))
x_query_np = np.ascontiguousarray(x_query.to_numpy(dtype=np.float32))


# Recall Measurement functions
truth_D, truth_I = None, None
def k_recall_at_k(test, truth, k=k):
    assert len(test) == k
    assert len(truth) == k
    return len(set(truth).intersection(test)) / k
def batch_recall(test_I, truth_I, k=k):
    results = []
    for i, row in enumerate(test_I):
        results.append(k_recall_at_k(row, truth_I[i], k))
        # print(results[-1])
    # return np.mean(results)
    return sum(results) / len(results)

#------------------------------------------

# Flat Index (Brute Force)
IFL2 = None
def brute_force_build():
    global IFL2;
    IFL2 = faiss.IndexFlatL2(d)
    IFL2.add(x_train)

def brute_force_search(k=k, measure_accuracy=True):
    global truth_D, truth_I
    truth_D, truth_I = IFL2.search(x_query, k) # D: Distances, I: Indices
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

# LSH Index
#LSH = None
#def lsh_build(n_bits=2*d):
 #   global LSH;
  #  LSH = faiss.IndexLSH (d, n_bits)
    # LSH.train (x_train) # LSH Technically doesn't require training
   # LSH.add (x_train) # NOTE: Can add other indexed data than what it's trained on

# PQ Index
#PQ = None
#def pq_build(subquantizers=10, n_bits=8):
 #   global PQ;
  #  PQ = faiss.IndexPQ(d, subquantizers, n_bits)
   # PQ.train(x_train)
    #PQ.add(x_train)

# IVF PQ Index
#IVFPQ = None
#def ivfpq_build(ncentroids=40, code_size=8, n_bits=8):
 #   # ncentroids = Number of Voronoi cells
    # code_size = Number of sub-vectors for product quantization
  #  global IVFPQ;
   # coarse_quantizer = faiss.IndexFlatL2(d)
   # IVFPQ = faiss.IndexIVFPQ(coarse_quantizer, d, ncentroids, code_size, n_bits)
   # IVFPQ.train(x_train)
   # IVFPQ.add(x_train)
   # IVFPQ.nprobe = 5

# HNSW Index
HNSW = None
def hnsw_build(data, dim, ef_construction=200, M=16, ef_search=100):
    global HNSW
    arr = x_train_np
    HNSW = hnswlib.Index(space='l2', dim=dim)
    HNSW.init_index(max_elements=arr.shape[0], ef_construction=ef_construction, M=M)
    HNSW.add_items(arr)
    HNSW.set_ef(ef_search)

def hnsw_search(k=k, measure_accuracy=True):
    global HNSW, truth_I
    labels, distances = HNSW.knn_query(x_query_np, k=k)
    if measure_accuracy:
        return batch_recall(labels, truth_I, k)
    return labels, distances


#------------------------------------------

#lsh_nbits        = [100, 200, 400, 800, 1600]
#pq_nbits         = [8, 12]#, 16] # With 16, we run into warnings about high cluster count compared to data
#pq_subquantizers = [5, 8, 10, 20, 25, 40] # How many pieces the vectors are split into
#         Commonly [8, 16, 32], but our vectors are not a power of 2
#ivfpq_ncentroids = [5, 8, 10, 20, 25, 40] # Same??
#ivfpq_codesize   = [4, 8, 10, 20, 25, 40] # OLD: [4, 8, 16, 32, 64] Is this actually centroids? It complained it needs d to be a factor of this.

#top_lsh_nbits = None
#top_pq_nbits = None
#top_pq_subq = None
#top_ivfpq_nbits = None
#top_ivfpq_ncentr = None
#top_ivfpq_csize = None

def test_suite(r=5): # r = number of repeats to get more accurate average timings
   # global top_lsh_nbits;
   # global top_pq_nbits;
   # global top_pq_subq;
   # global top_ivfpq_nbits;
   # global top_ivfpq_ncentr;
   # global top_ivfpq_csize;
    recall_str = f"{k}-recall@{k}"
    print(f"(All times averaged out of {r} repeats)")
    
    # Brute Force Ground Truth ----------------
    print("---------------------- Brute Force ----------------------")
    # brute_force_build()
    # brute_force_search()
    bf_btime = np.mean(repeat(lambda: brute_force_build(),  number=1, repeat=r))#, globals=globals()))
    bf_stime = np.mean(repeat(lambda: brute_force_search(), number=1, repeat=r))#, globals=globals()))

    print("Brute Force Accuracy: 1.0")
    print("Brute Force Build  Time:", bf_btime)
    print("Brute Force Search Time:", bf_stime)


    # LSH -------------------------------------
   # print("---------------------- LSH ----------------------")
   # lsh_results = []
   # lsh_btimes = []
   # lsh_stimes = []
   # for n in lsh_nbits:
        # lsh_build(n)
       # lsh_btimes.append(np.mean(repeat(lambda: lsh_build(n),  number=1, repeat=r)))
       # lsh_results.append(search(LSH, k))
       # lsh_stimes.append(np.mean(repeat(lambda: search(LSH, k, measure_accuracy=False),  number=1, repeat=r)))
   # lsh_results = np.array(lsh_results)
   # top_i = np.argmax(lsh_results)
   # top_lsh_nbits = lsh_nbits[top_i]
   # top_lsh_accuracy = lsh_results[top_i]

   # lsh_btimes = np.array(lsh_btimes)
   # fastb_i = np.argmin(lsh_btimes)
   # fastb_lsh_nbits = lsh_nbits[fastb_i]
   # fastb_lsh_time = lsh_btimes[fastb_i]
   # slowb_lsh_time = np.max(lsh_btimes)

   # lsh_stimes = np.array(lsh_stimes)
   # fasts_i = np.argmin(lsh_stimes)
   # fasts_lsh_nbits = lsh_nbits[fasts_i]
   # fasts_lsh_time = lsh_stimes[fasts_i]

   # print("LSH recalls:", lsh_results)
   # print(f"LSH best {recall_str}:", top_lsh_accuracy)
   # print("LSH best n_bits:", top_lsh_nbits)

   # print(f"LSH build time range: {fastb_lsh_time}-{slowb_lsh_time}, Fastest: {fastb_lsh_nbits} n_bits")
   # print("LSH search times:", lsh_stimes)
   # print(f"LSH fastest search: {fasts_lsh_nbits} n_bits, Time: {fasts_lsh_time}, Speedup: {bf_stime/fasts_lsh_time}")


    # PQ -------------------------------------
   # print("---------------------- PQ ----------------------")
   # pq_results = []
   # pq_btimes = []
   # pq_stimes = []
   # for n in pq_nbits:
       # pq_results.append([])
       # pq_btimes.append([])
       # pq_stimes.append([])
       # print("--- PQ n_bits:", n)
       # for m in pq_subquantizers:
           # print("--- --- PQ Subquantizers:", m)
            # pq_build(m, n)
           # pq_btimes[-1].append(np.mean(repeat(lambda: pq_build(m, n),  number=1, repeat=r)))
           # pq_results[-1].append(search(PQ, k))
           # pq_stimes[-1].append(np.mean(repeat(lambda: search(PQ, k, measure_accuracy=False),  number=1, repeat=r)))
   # pq_results = np.array(pq_results)
   # top_i = np.argmax(pq_results)
   # top_i = np.unravel_index(top_i, pq_results.shape)
   # top_pq_nbits = pq_nbits[top_i[0]]
   # top_pq_subq = pq_subquantizers[top_i[1]]
   # top_pq_accuracy = pq_results[top_i]

   # pq_btimes = np.array(pq_btimes)
   # fastb_i = np.argmin(pq_btimes)
   # fastb_i = np.unravel_index(fastb_i, pq_btimes.shape)
   # fastb_pq_nbits = pq_nbits[fastb_i[0]]
   # fastb_pq_subq = pq_subquantizers[fastb_i[1]]
   # fastb_pq_time = pq_btimes[fastb_i]
   # slowb_pq_time = np.max(pq_btimes)

   # pq_stimes = np.array(pq_stimes)
   # fasts_i = np.argmin(pq_stimes)
   # fasts_i = np.unravel_index(fasts_i, pq_stimes.shape)
   # fasts_pq_nbits = pq_nbits[fasts_i[0]]
   # fasts_pq_subq = pq_subquantizers[fasts_i[1]]
   # fasts_pq_time = pq_stimes[fasts_i]

   # print("PQ recalls:", pq_results)
   # print(f"PQ best {recall_str}:", top_pq_accuracy)
   # print("PQ best n_bits:", top_pq_nbits)
   # print("PQ best subqantizer:", top_pq_subq)

   # print(f"PQ build time range: {fastb_pq_time}-{slowb_pq_time}, Fastest: {fastb_pq_nbits} n_bits, {fastb_pq_subq} subquantizers")
   # print("PQ search times:", pq_stimes)
   # print(f"PQ fastest search: {fasts_pq_nbits} n_bits, {fasts_pq_subq} subquantizers; Time: {fasts_pq_time}, Speedup: {bf_stime/fasts_pq_time}")


    # IVF PQ -------------------------------------
   # print("---------------------- IVF PQ ----------------------")
   # ivfpq_results = []
   # ivfpq_btimes = []
   # ivfpq_stimes = []
   # for n in pq_nbits:
      #  ivfpq_results.append([])
      #  ivfpq_btimes.append([])
      #  ivfpq_stimes.append([])
      #  print("--- IVF PQ n_bits:", n)
      #  for m in ivfpq_ncentroids:
        #    ivfpq_results[-1].append([])
         #   ivfpq_btimes[-1].append([])
          #  ivfpq_stimes[-1].append([])
           # print("--- --- IVF PQ ncentroids:", m)
           # for p in ivfpq_codesize:
               # print("--- --- --- IVF PQ codesize:", p)
                # ivfpq_build(m, p, n)
               # ivfpq_btimes[-1][-1].append(np.mean(repeat(lambda: ivfpq_build(m, p, n),  number=1, repeat=r)))
               # ivfpq_results[-1][-1].append(search(IVFPQ, k))
               # ivfpq_stimes[-1][-1].append(np.mean(repeat(lambda: search(IVFPQ, k, measure_accuracy=False),  number=1, repeat=r)))
   # ivfpq_results = np.array(ivfpq_results)
   # top_i = np.argmax(ivfpq_results)
   # top_i = np.unravel_index(top_i, ivfpq_results.shape)
   # top_ivfpq_nbits = pq_nbits[top_i[0]]
   # top_ivfpq_ncentr = ivfpq_ncentroids[top_i[1]]
   # top_ivfpq_csize = ivfpq_codesize[top_i[2]]
   # top_ivfpq_accuracy = ivfpq_results[top_i]
    
   # ivfpq_btimes = np.array(ivfpq_btimes)
   # fastb_i = np.argmin(ivfpq_btimes)
   # fastb_i = np.unravel_index(fastb_i, ivfpq_btimes.shape)
   # fastb_ivfpq_nbits = pq_nbits[fastb_i[0]]
   # fastb_ivfpq_ncentr = ivfpq_ncentroids[fastb_i[1]]
   # fastb_ivfpq_csize = ivfpq_codesize[fastb_i[2]]
   # fastb_ivfpq_time = ivfpq_btimes[fastb_i]
   # slowb_ivfpq_time = np.max(ivfpq_btimes)
    
   # ivfpq_stimes = np.array(ivfpq_stimes)
   # fasts_i = np.argmin(ivfpq_stimes)
   # fasts_i = np.unravel_index(fasts_i, ivfpq_stimes.shape)
   # fasts_ivfpq_nbits = pq_nbits[fasts_i[0]]
   # fasts_ivfpq_ncentr = ivfpq_ncentroids[fasts_i[1]]
   # fasts_ivfpq_csize = ivfpq_codesize[fasts_i[2]]
   # fasts_ivfpq_time = ivfpq_stimes[fasts_i]

   # print("IVFPQ recalls:", ivfpq_results)
   # print(f"IVFPQ best {recall_str}:", top_ivfpq_accuracy)
   # print("IVFPQ best n_bits:", top_ivfpq_nbits)
   # print("IVFPQ best ncentroids:", top_ivfpq_ncentr)
   # print("IVFPQ best code_size:", top_ivfpq_csize)

   # print(f"IVFPQ build time range: {fastb_ivfpq_time}-{slowb_ivfpq_time}, Fastest: {fastb_ivfpq_nbits} n_bits, {fastb_ivfpq_ncentr} ncentroids, {fastb_ivfpq_csize} codesize")
   # print("IVFPQ search times:", ivfpq_stimes)
   # print(f"IVFPQ fastest search: {fasts_ivfpq_nbits} n_bits, {fasts_ivfpq_ncentr} ncentroids, {fasts_ivfpq_csize} codesize; Time: {fasts_ivfpq_time}, Speedup: {bf_stime/fasts_ivfpq_time}")

    # HNSW -------------------------------------
    print("---------------------- HNSW ----------------------")
    # time the build
    hnsw_btime = np.mean(repeat(lambda: hnsw_build(x_train, d, ef_construction=200, M=16, ef_search=100), number=1, repeat=r))
    # accuracy at current k
    hnsw_result = hnsw_search(k)
    # time the search
    hnsw_stime = np.mean(repeat(lambda: hnsw_search(k, measure_accuracy=False), number=1, repeat=r))

    print("HNSW recall:", hnsw_result)
    print(f"HNSW build time: {hnsw_btime}")
    print("HNSW search time:", hnsw_stime)
    print(f"HNSW search Speedup: {bf_stime/hnsw_stime}")


    # All -------------------------------------
   # print("---------------------- ALL ----------------------")
   # indexes = ["LSH", "PQ", "IVFPQ", "HNSW"]
   # best_times = [fasts_lsh_time, fasts_pq_time, fasts_ivfpq_time]
   # best_timeb = [fastb_lsh_time, fastb_pq_time, fastb_ivfpq_time]
   # best_accuracy = [top_lsh_accuracy, top_pq_accuracy, top_ivfpq_accuracy]
   # best_i = np.argmax(best_accuracy)
   # fastb_i = np.argmin(best_timeb)
   # fasts_i = np.argmin(best_times)

   # print("Most Accurate:", indexes[best_i])
   # print("Fastest Build:", indexes[fastb_i])
   # print("Fastest Search:", indexes[fasts_i])



if __name__ == "__main__":
    test_suite()

"""
from datasketch import MinHash
m = MinHash()
for d in df:
    m.update(d.encode('utf8'))
print("Estimated Jaccard for m and m2:", m.jaccard(m2))
# More: https://ekzhu.com/datasketch/minhash.html
"""
