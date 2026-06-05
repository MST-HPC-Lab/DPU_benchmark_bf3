#Read from the json

import json
from pathlib import Path

import matplotlib
import matplotlib.pyplot as plt
import numpy as np

RESULTS_JSON = "results/results.json"
DATASET = "glove6B.200d.txt"
DIMENSIONS = 200
NUM_VECS = 400000
THREADS = "default"
K_FOR_SCATTER = 10 
OUT_DIR = Path("results")

def load_results(json_path):
    with open(json_path, "r", encoding="utf-8") as f:
        return json.load(f)

def get_kvaried_run(
        results,
        device,
        dataset=DATASET,
        dimensions=DIMENSIONS,
        num_vecs=NUM_VECS,
        threads=THREADS,
    ):
    return results[str(device)][str(dataset)][str(dimensions)][str(num_vecs)][str(threads)]

def get_multitest_run(results, device, dataset=DATASET, k=K_FOR_SCATTER):
    return results["multitest"][str(device)][str(dataset)][str(k)]

def as_array(value):
    return np.asarray(value, dtype=float).ravel()

results = load_results(RESULTS_JSON)

host = get_kvaried_run(
    results,
    device = "host",
    dataset = DATASET,
    dimensions = DIMENSIONS,
    num_vecs = NUM_VECS,
    threads = THREADS,
)




k = as_array(host["k_values"])
host_lsh_recall = as_array(host["lsh_recalls"])
host_pq_recall = as_array(host["pq_reccalls"])
host_ivfpq_recall = as_array(host["ivfpq_recalls"])
host_hnsw_recall = as_array(host["hnsw_reccall"])





#end for read from json


#speed_glove

# GRAPHS :
plt.title("Time Cost of Recall Across Methods (Host; varying parameters)")
plt.grid(color='0.8')
# plt.scatter(host_IVFPQ_k10_recall, host_IVFPQ_k10_time, color='orange')
# plt.scatter(host_PQ_k10_recall, host_PQ_k10_time, color='teal')
# plt.scatter(host_LSH_k10_recall, host_LSH_k10_time, color='purple')

# plt.ylabel("Time (ms)")
#To do: Check if 1000 is correct
plt.scatter(host_IVFPQ_k10_recall, 1000/host_IVFPQ_k10_time, color='orange')
plt.scatter(host_PQ_k10_recall, 1000/host_PQ_k10_time, color='teal')
plt.scatter(host_LSH_k10_recall, 1000/host_LSH_k10_time, color='purple')
plt.scatter(host_HNSW_k10_recall, 1000/host_HNSW_k10_time, color='navy')
plt.ylabel("Queries per Second (1/s)")
plt.xlabel("10-Recall@10")
plt.xlim(0, 1.1)
plt.legend(["IVFPQ", "PQ", "LSH", "HNSW"])
# plt.show()#block=False)
plt.savefig("results/recall_vs_speed.png")
plt.close()

# Logarithmic Graph:
plt.title("Time Cost of Recall Across Methods (Host; varying parameters)")
plt.grid(color='0.8')
plt.loglog(host_IVFPQ_k10_recall, 1000/host_IVFPQ_k10_time, 'o', color='orange')
plt.loglog(host_PQ_k10_recall, 1000/host_PQ_k10_time, 'o', color='teal')
plt.loglog(host_LSH_k10_recall, 1000/host_LSH_k10_time, 'o', color='purple')
plt.loglog(host_HNSW_k10_recall, 1000/host_HNSW_k10_time, 'o', color='navy')
plt.ylabel("Queries per Second (1/s)")
plt.xlabel("10-Recall@10")
# plt.xlim(0, 1.1)
plt.legend(["IVFPQ", "PQ", "LSH", "HNSW"])
# plt.show()#block=False)
plt.savefig("results/recall_vs_speed_logarithmic.png")
plt.close()



#k_glove 


matplotlib.rcParams['font.family'] = 'DejaVu Serif'

matplotlib.rcParams['font.size'] = 12
matplotlib.rcParams['axes.titlesize'] = 14
matplotlib.rcParams['axes.labelsize'] = 12
matplotlib.rcParams['legend.fontsize'] = 6

matplotlib.rcParams['xtick.labelsize'] = 11
matplotlib.rcParams['ytick.labelsize'] = 11
matplotlib.rcParams['axes.linewidth'] = 1.2
matplotlib.rcParams['legend.frameon'] = True

# GRAPHS :
#1. Recall vs. K (Host vs. BF3)
plt.figure(figsize=(9,4))
plt.title("Recall vs. k (GloVe Dataset, Host vs. BF3)")
#plt.title("glove.6B.200d.txt: How Recall Varies with k\n(other parameters chosen so that recall is 100% when k=1)")
plt.grid(True, linestyle='--', alpha=0.6)

#Host (Solid Lines)

plt.plot(k, host_lsh_recall, color='purple', label='Host LSH', linewidth=1)
plt.plot(k, host_pq_recall, color='teal', label='Host PQ', linewidth=1)
plt.plot(k, host_ivfpq_recall, color='orange', label='Host IVFPQ', linewidth=1)
plt.plot(k, host_hnsw_recall, color='navy', label='Host HNSW', linewidth=1)

#BF3 (Dotted lines with markers)
plt.plot(k, bf3_lsh_recall, 'o:', color='purple', label='BF3 LSH',linewidth=1)
plt.plot(k, bf3_pq_recall, 'o:', color='teal', label='BF3 PQ', linewidth=1)
plt.plot(k, bf3_ivfpq_recall, 'o:', color='orange', label='BF3 IVFPQ', linewidth=1)
plt.plot(k, bf3_hnsw_recall, 'o:', color='navy', label='BF3 HNSW', linewidth=1)

#plt.plot(k, host_lsh_recall   ,       color='purple')
#plt.plot(k, host_pq_recall    ,       color='teal')
#plt.plot(k, host_ivfpq_recall ,       color='orange')
#plt.plot(k, host_hnsw_recall ,        color ='navy')
# plt.plot(k, bf3_lsh_recall    , 'o:', color='purple')
# plt.plot(k, bf3_pq_recall     , 'o:', color='teal')
# plt.plot(k, bf3_ivfpq_recall  , 'o:', color='orange')

plt.xscale('log')
plt.ylabel("Recall@k")
plt.xlabel("k")
plt.ylim(0, 1.1)
plt.legend(
    loc='lower left',
    bbox_to_anchor=(1.35, 0),
    fontsize=7,
    frameon=True,
    fancybox=True,
    framealpha=1,
    ncol=1
)

plt.tight_layout(rect=[0, 0, 0.99, 1])
#plt.legend(["LSH", "PQ", "IVFPQ", "HNSW"]) #["Host LSH", "Host PQ", "Host IVFPQ", "BF3 LSH", "BF3 PQ", "BF3 IVFPQ"]
# plt.show()#block=False)
#plt.savefig("results/recall_vs_k.png")
#plt.savefig("fulldataset_glove_recall_vs_k.png")
plt.savefig("glove_recall_vs_k_host_vs_bf3.png", dpi=300, bbox_inches='tight') 
plt.close()



#2. Time vs. K (Host vs. BF3)
plt.figure(figsize=(9,4))
#plt.title("glove.6B.200d.txt: How Recall Time Varies with k\n(other parameters chosen so that recall is 100% when k=1)")
plt.title("Time vs. k (GloVe Dataset, Host vs. BF3)") 
plt.grid(True, linestyle='--', alpha=0.6)

#Host (Solid Lines)
plt.plot(k, host_bf_time, color='black', label='Host Flat',linewidth=1)
#plt.plot(k, host_bf_time, color='black', linestyle='--', alpha=0.6, label='Host Flat')
plt.plot(k, host_lsh_time, color='purple', label='Host LSH',linewidth=1)
plt.plot(k, host_pq_time, color='teal', label='Host PQ',linewidth=1)
plt.plot(k, host_ivfpq_time, color='orange', label='Host IVFPQ',linewidth=1)
plt.plot(k, host_hnsw_time, color='navy', label='Host HNSW',linewidth=1)

#BF3 (Dotted lines with markers)
plt.plot(k, bf3_bf_time, 'o:', color='black', alpha=0.6, label='BF3 Flat',linewidth=1)
#plt.plot(k, bf3_bf_time, 'o:', color='black', label='BF3 Flat',linewidth=2)
plt.plot(k, bf3_lsh_time, 'o:', color='purple', label='BF3 LSH',linewidth=1)
plt.plot(k, bf3_pq_time, 'o:', color='teal', label='BF3 PQ',linewidth=1)
plt.plot(k, bf3_ivfpq_time, 'o:', color='orange', label='BF3 IVFPQ',linewidth=1)
plt.plot(k, bf3_hnsw_time, 'o:', color='navy', label='BF3 HNSW',linewidth=1)

# plt.scatter(host_IVFPQ_k10_recall, host_IVFPQ_k10_time, color='orange')
# plt.scatter(host_PQ_k10_recall, host_PQ_k10_time, color='teal')
# plt.scatter(host_LSH_k10_recall, host_LSH_k10_time, color='purple')
# plt.ylabel("Time (ms)")
#plt.plot(k, host_bf_time    ,       color='k')
#plt.plot(k, host_lsh_time   ,       color='purple')
#plt.plot(k, host_pq_time    ,       color='teal')
#plt.plot(k, host_ivfpq_time ,       color='orange')
#plt.plot(k, host_hnsw_time ,       color='navy')
#plt.plot(k, bf3_bf_time     , 'o:', color='k')
#plt.plot(k, bf3_lsh_time    , 'o:', color='purple')
#plt.plot(k, bf3_pq_time     , 'o:', color='teal')
#plt.plot(k, bf3_ivfpq_time  , 'o:', color='orange')
#plt.plot(k, bf3_hnsw_time  , 'o:', color='navy')
#plt.xscale('log')
plt.ylabel("Time (sec)")
plt.xlabel("k")
#plt.ylim(bottom=0)
plt.ylim(0.1, 7)
plt.yscale('log')
plt.legend(
    loc='lower left',
    bbox_to_anchor=(1.35, 0),
    fontsize=7,
    frameon=True,
    fancybox=True,
    framealpha=1,
    ncol=1
)

plt.tight_layout(rect=[0, 0, 1.02, 1])
#plt.legend(["Host Flat", "Host LSH", "Host PQ", "Host IVFPQ", "Host HNSW", "BF3 Flat", "BF3 LSH", "BF3 PQ", "BF3 IVFPQ", "BF3 HNSW"])
# plt.show()#block=False)
#plt.savefig("results/recalltime_vs_k.png")
#plt.savefig("fulldataset_glove_recalltime_vs_k.png")
plt.savefig("glove_time_vs_k_host_vs_bf3.png", dpi=300, bbox_inches='tight')
plt.tight_layout()
plt.close()

'''
#3. Recall vs. Time (Host) — SCATTER
plt.figure(figsize=(6, 4))
plt.title("Recall vs Time (GloVe Dataset, Host)")
plt.grid(True, linestyle='--', alpha=0.6)

# Plot each algorithm (scatter only)
plt.scatter(host_lsh_time, host_lsh_recall, color='purple', label='LSH', s=60)
plt.scatter(host_pq_time, host_pq_recall, color='teal', label='PQ', s=60)
plt.scatter(host_ivfpq_time, host_ivfpq_recall, color='orange', label='IVFPQ', s=60)
plt.scatter(host_hnsw_time, host_hnsw_recall, color='navy', label='HNSW', s=60)

# Axis scaling
all_host_times = np.concatenate([host_lsh_time, host_pq_time, host_ivfpq_time, host_hnsw_time])
plt.xlim(all_host_times.min() * 0.9, all_host_times.max() * 1.1)
plt.ylim(0.35, 1.01)

plt.xlabel("Time (sec)")
plt.ylabel("Recall")

plt.legend(
    loc='lower left',
    bbox_to_anchor=(1.2, 0),
    fontsize=7,
    frameon=True,
    fancybox=True,
    framealpha=1,
    ncol=1
)

plt.tight_layout(rect=[0, 0, 0.82, 1])
plt.tight_layout()
plt.savefig("glove_recall_vs_time_host_scatter.png", dpi=300, bbox_inches='tight')
plt.close()


#4. Recall vs. Time (BF3) — SCATTER
plt.figure(figsize=(6, 4))
plt.title("Recall vs Time (GloVe Dataset, BF3)")
plt.grid(True, linestyle='--', alpha=0.6)

# Plot each algorithm
plt.scatter(bf3_lsh_time, bf3_lsh_recall, color='purple', label='LSH', s=60)
plt.scatter(bf3_pq_time, bf3_pq_recall, color='teal', label='PQ', s=60)
plt.scatter(bf3_ivfpq_time, bf3_ivfpq_recall, color='orange', label='IVFPQ', s=60)
plt.scatter(bf3_hnsw_time, bf3_hnsw_recall, color='navy', label='HNSW', s=60)

# Axis scaling
all_bf3_times = np.concatenate([bf3_lsh_time, bf3_pq_time, bf3_ivfpq_time, bf3_hnsw_time])
plt.xlim(all_bf3_times.min() * 0.9, all_bf3_times.max() * 1.1)
plt.ylim(0.35, 1.01)

plt.xlabel("Time (sec)")
plt.ylabel("Recall")

plt.legend(
    loc='lower left',
    bbox_to_anchor=(1.09, 0),
    fontsize=7,
    frameon=True,
    fancybox=True,
    framealpha=1,
    ncol=1
)

plt.tight_layout(rect=[0, 0, 0.99, 1])
plt.tight_layout()
plt.savefig("glove_recall_vs_time_bf3_scatter.png", dpi=300, bbox_inches='tight')
plt.close()
'''