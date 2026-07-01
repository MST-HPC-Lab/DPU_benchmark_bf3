#Read from the json

import json
from pathlib import Path
import numpy as np

import matplotlib
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D


RESULTS_HOST = "results/results_host.json"
RESULTS_BF3 = "results/results_bf3.json"
GLOVE = "glove.6B.200d.txt"
FASTTEXT = "fasttext_cc.en.300.vec"
SIFT = "sift10m.mat"
K_FOR_SCATTER = 10 
OUT_DIR = Path("results")
DEFAULT_THREADS = "default"
ALG_RECALLS = ["flat_recalls", "lsh_recalls", "pq_recalls", "ivfpq_recalls", "hnsw_recalls", "hnsw_pq_recalls", "hnsw_sq_recalls"]
ALG_TIMES = ["flat_times", "lsh_times", "pq_times", "ivfpq_times", "hnsw_times", "hnsw_pq_times", "hnsw_sq_times"]
ALG_NAMES = ["Flat", "LSH", "PQ", "IVFPQ", "HNSW", "HNSW-PQ", "HNSW-SQ"]
ALG_COLORS = ['black', 'green', 'red', 'orange', 'blue', 'purple', 'teal']
GLOVE_D = 200
FASTTEXT_D = 300
SIFT_D = 128
EQUALIZED_D = 128
GLOVE_N = 400000
FASTTEXT_N = 2000000
SIFT_N = 11164866
EQUALIZED_N = 400000


    
# Load results
def load_results(json_path):
    with open(json_path, "r", encoding="utf-8") as f:
        return json.load(f)
host_results = load_results(RESULTS_HOST)
bf3_results = load_results(RESULTS_BF3)
# Combine into one JSON for convenience
results = host_results
results["bf3"] = bf3_results["bf3"]
if "multitest" in bf3_results:
    results["multitest"]["bf3"] = bf3_results["multitest"]["bf3"]

# Define helper functions
def as_array(value):
    return np.asarray(value, dtype=float).ravel()

def get_kvaried_run(
        device,
        dataset,
        dimensions,
        num_vecs,
        threads,
        results=results,
    ):
    r = results[str(device)][str(dataset)][str(dimensions)][str(num_vecs)][str(threads)]
    # Convert results to arrays
    for alg in ALG_RECALLS + ALG_TIMES:
        if alg in r:
            r[alg] = as_array(r[alg])
    r["k_values"] = np.asarray(r["k_values"], dtype=int)#.ravel()
    return r

def get_multitest_run(dataset, device="host", k=K_FOR_SCATTER, results=results):
    return results["multitest"][str(device)][str(dataset)][str(k)]

def per_query_time(times, num_queries):
    return times / num_queries

def queries_per_second(times, num_queries):
    return num_queries / times

def setup_plot(title, xlabel=None, ylabel=None, legend=None, xscale='linear', yscale='linear', xlim=None, ylim=None, xticks=None):
    plt.title(title)
    plt.grid(True, alpha=0.6)
    if xlabel is not None: plt.xlabel(xlabel)
    if ylabel is not None: plt.ylabel(ylabel)
    plt.xscale(xscale)
    plt.yscale(yscale)
    if legend is not None:
        if legend is not False:
            plt.legend(legend,
                loc='best',
                # bbox_to_anchor=(1.35, 0),
                fontsize=7,
                frameon=True,
                fancybox=True,
                framealpha=1,
                ncol=1)
    else: plt.legend()
    if xlim is not None:
        plt.xlim(xlim)
    if ylim is not None:
        if ylim is True:
            plt.ylim(bottom=0)
        else:
            plt.ylim(ylim)
    if xticks is not None:
        plt.xticks(xticks, labels=xticks)#, rotation=45, fontsize=10)

def finish_plot(filename):
    plt.tight_layout()
    plt.savefig(OUT_DIR / filename, dpi=300, bbox_inches='tight')
    plt.close()

def make_legend_lines(colors, alphas, marks):
    # NOTE: Legend names beginning with "_" are excluded, so this function will not always be needed.
    return [Line2D([0], [0], color=c, alpha=a, lw=2, linestyle=m) for c, a, m in zip(colors, alphas, marks)]


#============= GRAPHS =========================================================#


#------------- Alg Recall vs. K -------------------#
rs   = get_kvaried_run("host", SIFT,     SIFT_D,      SIFT_N,      DEFAULT_THREADS)
rf   = get_kvaried_run("host", FASTTEXT, FASTTEXT_D,  FASTTEXT_N,  DEFAULT_THREADS)
rg   = get_kvaried_run("host", GLOVE,    GLOVE_D,     GLOVE_N,     DEFAULT_THREADS)
k  = rg["k_values"]

# Setup Custom Legend
legend_names = ["SIFT", "FastText", "GloVe", ""] + ALG_NAMES.copy()[1:]
legend_colors = ["black", "black", "black", "black"] + ALG_COLORS.copy()[1:]
# legend_alphas = [1, 0.5, 0.25, 0.0] + [1] * (len(ALG_NAMES)-1)
legend_alphas = [1, 1, 1, 0] + [1] * (len(ALG_NAMES)-1)
legend_marks = ['-', '--', ':', '-'] + ['-'] * (len(ALG_NAMES)-1)
legend_lines = make_legend_lines(legend_colors, legend_alphas, legend_marks)
plt.legend(legend_lines, legend_names, bbox_to_anchor=(1.35, 1))

# Main Plotting
setup_plot(
    title="Algorithm Recall vs. k\n(Device-Independent)",
    xlabel="k", ylabel="Recall@k",
    xticks=k, xscale='log', ylim=True, legend=False,
)
# for dataset, eq, alpha, mark, m in ((rs, rs_E, 1, '-', 'o'), (rf, rf_E, 1, '--', 'D'), (rg, rg_E, 1, ':', '*')):
# for dataset, alpha, mark in ((rs, 1, '-'), (rf, 0.5, '--'), (rg, 0.25, ':')):
for dataset, alpha, mark in ((rs, 1, '-'), (rf, 1, '--'), (rg, 1, ':')):
    for alg, color in zip(ALG_RECALLS[1:], ALG_COLORS[1:]):
        # try:
        plt.plot(k, dataset[alg], color=color, alpha=alpha, linestyle=mark)
        # plt.plot([10], eq[alg], color=color, alpha=alpha, linestyle=mark, marker=m, markersize=3)
        # except KeyError:
        #     pass
finish_plot("recall_vs_k.png")


#------------- Alg Recall vs. K (Dataset-Size-Equalized) --------#
rs_E = get_kvaried_run("host", SIFT,     EQUALIZED_D, EQUALIZED_N, DEFAULT_THREADS)
rf_E = get_kvaried_run("host", FASTTEXT, EQUALIZED_D, EQUALIZED_N, DEFAULT_THREADS)
rg_E = get_kvaried_run("host", GLOVE,    EQUALIZED_D, EQUALIZED_N, DEFAULT_THREADS)

hatches = ["", "////", 'xxxx']
x = np.arange(len(ALG_NAMES)-1)  # the label locations
width = 0.25  # the width of the bars
fig, ax = plt.subplots(figsize=(6, 4))
plt.title("Recall Comparison on Equalized Datasets\n(Device-Independent)")
plt.grid(True, axis='y', alpha=0.6)
bars1 = ax.bar(x - width, [rs_E[alg][-1] for alg in ALG_RECALLS[1:]], width*.8, label='SIFT', color=ALG_COLORS[1:], hatch=hatches[0], edgecolor='white')
bars2 = ax.bar(x, [rf_E[alg][-1] for alg in ALG_RECALLS[1:]], width*.8, label='FastText', color=ALG_COLORS[1:], hatch=hatches[1], edgecolor='white')
bars3 = ax.bar(x + width, [rg_E[alg][-1] for alg in ALG_RECALLS[1:]], width*.8, label='GloVe', color=ALG_COLORS[1:], hatch=hatches[2], edgecolor='white')
ax.set_xticks(x)
ax.set_xticklabels(ALG_NAMES[1:])
ax.set_ylabel("10-Recall@10")
ax.set_ylim(0, 1)
plt.legend(loc='upper left')
finish_plot("recall_vs_k_equalized.png")


#------------- Alg Recall vs. K (Data Scaling) --------#
x = [100000, 200000, 500000, 1000000, 2000000, 5000000, 10000000]
x_labels = ["100k", "200k", "500k", "1M", "2M", "5M", "10M"]
rs_scale = [get_kvaried_run("host", SIFT, SIFT_D, n, DEFAULT_THREADS) for n in x]
for r in rs_scale:
    assert r["k_values"][0] == 10, "Expected k=10 for all runs"
for alg, color, name in zip(ALG_RECALLS[1:], ALG_COLORS[1:], ALG_NAMES[1:]):
    plt.plot(x, [r[alg][0] for r in rs_scale], color=color, label=name)
setup_plot(
    title="Algorithm Recall vs. Dataset Size\n(Device-Independent)",
    xlabel="Subset Size of SIFT", ylabel="10-Recall@10",
    xscale='log', ylim=True, legend=False,
)
plt.xticks(x, labels=x_labels)
plt.legend(loc='lower left')
finish_plot("recall_vs_dataset_size.png")


#------------- Time Cost of Recall --------#


#------------- Query Time vs. K (Per Dataset) --------#


#------------- Query Time vs. K (16-Thread-Equalized, Per Dataset) --------#


#------------- Query Time vs. Threads (Per Dataset) --------#


#------------- Throughput vs. Dataset Size --------#


#------------- Throughput vs. Dataset Size (16-Thread-Equalized) --------#




#============= OLD ============================================================#

'''
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