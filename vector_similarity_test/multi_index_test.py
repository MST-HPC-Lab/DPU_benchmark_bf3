import pandas as pd
import numpy as np
import faiss
from timeit import repeat

import index_builder as ib


# Load Data File (TODO: make this more flexible for different datasets)
# df = pd.read_csv("../Data/glove.6B.200d.txt", sep=" ", quoting=3, header=None)
# d = 300
df = pd.read_csv("../Data/glove.6B.200d.txt", sep=" ", quoting=3, skiprows=1, header=None)
truth_path = "indices/glove_clean/truth_I,D.json"
d = 200


k = 10 # Number of nearest neighbors to search for (TODO: update this and graph to 100?)

# Split into vocab column and data
# vocab = df.iloc[:, 0]
df = df.drop(0, axis=1)

# Fixed Split
N = len(df)
test_i = [i for i in range(d-1, N, d)]
train_i = [i for i in range(N) if (i + d-1) % d]

ib.x_query = df.iloc[test_i]
ib.x_query = np.ascontiguousarray(ib.x_query.to_numpy(dtype=np.float32))
ib.x_train = df.iloc[train_i]
ib.x_train = np.ascontiguousarray(ib.x_train.to_numpy(dtype=np.float32))
del df


# Timing
def avg_time(fn, reps=3):
    fn()
    return np.mean(repeat(fn, number=1, repeat=reps))


# TEST
def test_suite(r=3):
    print(f"(All times averaged over {r} repeats)")
    ib.load_truth(truth_path)

    # Ground Truth or Brute Force
    print("\nBrute Force")
    # bf_build = avg_time(lambda: ib.brute_force_build(x_train), r)
    # brute_force_build()
    bf_search = avg_time(lambda: ib.brute_force_search(k, measure_accuracy=False), r)
    # print("Build Time:", bf_build)
    print("Search Time:", bf_search)
    # ib.x_train = None # Needed for other builds

    # Flat
    print("\nFlat")
    ib.flat_build(ib.x_train) # bt = avg_time(lambda: ib.flat_build(ib.x_train), r)
    flat_search = avg_time(lambda: ib.search(ib.FL2, k, measure_accuracy=False), r)
    # print("Build Time:", bt)
    print("Search Time:", flat_search)
    ib.FL2 = None

    # LSH 
    print("\nLSH")
    for nbits in [32, 64, 128, 256, 512, 1024, 1600]:
        ib.lsh_build(ib.x_train, nbits) # bt = avg_time(lambda: ib.lsh_build(ib.x_train, nbits), r)
        # lsh_build(nbits)
        # brute_force_search(k)

        rc = ib.search(ib.LSH, k)
        st = avg_time(lambda: ib.search(ib.LSH, k, measure_accuracy=False), r)
        print(f"nbits={nbits} | recall={rc:.3f}, time={st:.4f}")
        ib.LSH = None

    # PQ
    print("\nPQ")
    assert d == 200 or d == 300, "This test suite is designed for d=200 or d=300. Please adjust pq_m_vals accordingly if using a different dimension. They must be factors of d."
    pq_m_vals = [4, 5, 10, 20, 40] if d == 200 else [4, 5, 10, 25, 30, 50] if d == 300 else None # "subquantizers" or "m" in the PQ index, which is the number of subvectors the original vector is split into. It must be a factor of d.
    pq_n_bits = [4, 6, 8] # "nbits_per_index" or "nbits" in the PQ index, which is the number of bits used to encode each subvector.
    for m in pq_m_vals:
        for nbits in pq_n_bits:
            ib.pq_build(ib.x_train, m, nbits) # bt = avg_time(lambda: ib.pq_build(ib.x_train, m), r)
            # pq_build(m)
            # brute_force_search(k)

            rc = ib.search(ib.PQ, k)
            st = avg_time(lambda: ib.search(ib.PQ, k, measure_accuracy=False), r)
            print(f"m={m} | recall={rc:.3f}, time={st:.4f}")
            ib.PQ = None

    # IVFPQ 
    print("\nIVFPQ")
    nlist = [int(4 * np.sqrt(len(ib.x_train))), int(8 * np.sqrt(len(ib.x_train))), int(16 * np.sqrt(len(ib.x_train)))] # "nlist" or "ncentroids" in the IVFPQ index, which is the number of Voronoi cells (or clusters) used to partition the training data. It must be less than the number of training vectors.
    for ncentroids in nlist: # Usually ranges from 4xsqrt(|x_train|) to 16xsqrt(|x_train|).
        for code_size in pq_m_vals: # code_size and m are same if n_bits is 8, since code_size = (m * n_bits) / 8
            ib.ivfpq_build(ib.x_train, ncentroids, code_size, n_bits=8) #bt = avg_time(lambda: ib.ivfpq_build(ib.x_train, ncentroids, code_size, n_bits=8), r)
            ib.IVFPQ.nprobe = 32
            # ivfpq_build(nlist)
            # brute_force_search(k)

            rc = ib.search(ib.IVFPQ, k)
            st = avg_time(lambda: ib.search(ib.IVFPQ, k, measure_accuracy=False), r)
            print(f"nlist={nlist} | recall={rc:.3f}, time={st:.4f}")
            ib.IVFPQ = None

    # HNSW 
    print("\nHNSW")
    for M in [8, 16, 32]:
        for efc in [128, 200]: # Default is 40, but users find this range is the sweet spot. Should be significantly higher than M.
            ib.hnsw_build(ib.x_train, d, efc, M, efs=128) # bt = avg_time(lambda: ib.hnsw_build(ib.x_train, d, efc, M, efs=100), r)
            for efs in [128, 200]: # A good starting place is 2xk to 4xk, but it can be higher to improve recall.
                ib.HNSW.hnsw.efSearch = efs
                # hnsw_build(M, efc, efs)
                # brute_force_search(k) # truth_I calculation no longer needed; loaded from truth_I,D.json

                rc = ib.search(ib.HNSW, k)
                st = avg_time(lambda: ib.search(ib.HNSW, k, measure_accuracy=False), r)
                print(f"M={M}, efc={efc}, efs={efs} | recall={rc:.3f}, time={st:.4f}")
                ib.HNSW = None

    # HNSW+PQ
    print("\nHNSW+PQ")
    for M in [8, 16, 32]:
        for efc in [128, 200]: # Default is 40, but users find this range is the sweet spot. Should be significantly higher than M.
            for pq_m in pq_m_vals:
                ib.hnsw_pq_build(ib.x_train, d, efc, M, pq_m, efs=128) #bt = avg_time(lambda: ib.hnsw_pq_build(ib.x_train, M, efc, efs), r)
                for efs in [100, 200]: # A good starting place is 2xk to 4xk, but it can be higher to improve recall.
                    ib.HNSWPQ.hnsw.efSearch = efs
                    # hnsw_build(M, efc, efs)
                    # brute_force_search(k)

                    rc = ib.search(ib.HNSWPQ, k)
                    st = avg_time(lambda: ib.search(ib.HNSWPQ, k, measure_accuracy=False), r)
                    print(f"M={M}, efc={efc}, efs={efs}, pq_m={pq_m} | recall={rc:.3f}, time={st:.4f}")
                    ib.HNSWPQ = None

    # HNSW+SQ
    print("\nHNSW+SQ")
    for M in [8, 16, 32]:
        for efc in [128, 200]: # Default is 40, but users find this range is the sweet spot. Should be significantly higher than M.
            for sq in [faiss.ScalarQuantizer.QT_4bit, faiss.ScalarQuantizer.QT_8bit]:
                ib.hnsw_sq_build(ib.x_train, d, efc, M, sq, efs=128) #bt = avg_time(lambda: ib.hnsw_sq_build(ib.x_train, M, efc, efs), r)
                for efs in [100, 200]: # A good starting place is 2xk to 4xk, but it can be higher to improve recall.
                    ib.HNSWSQ.hnsw.efSearch = efs
                    # hnsw_sq_build(M, efc, efs)
                    # brute_force_search(k)

                    rc = ib.search(ib.HNSWSQ, k)
                    st = avg_time(lambda: ib.search(ib.HNSWSQ, k, measure_accuracy=False), r)
                    print(f"M={M}, efc={efc}, efs={efs}, sq_type={sq} | recall={rc:.3f}, time={st:.4f}")
                    ib.HNSWSQ = None


if __name__ == "__main__":
    test_suite()
    # TODO: Make it write to the json file!



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