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
DATASET_NAMES = ["SIFT", "FastText", "GloVe"]
K_FOR_SCATTER = 10 
DEFAULT_K = 10
OUT_DIR = Path("results")
DEFAULT_THREADS = "default"
ALG_RECALLS = ["flat_recalls", "lsh_recalls", "pq_recalls", "ivfpq_recalls", "hnsw_recalls", "hnsw_pq_recalls", "hnsw_sq_recalls"]
ALG_TIMES   = ["flat_times",   "lsh_times",   "pq_times",   "ivfpq_times",   "hnsw_times",   "hnsw_pq_times",   "hnsw_sq_times"  ]
ALG_NAMES   = ["Flat",         "LSH",         "PQ",         "IVFPQ",         "HNSW",         "HNSW-PQ",         "HNSW-SQ"        ]
ALG_COLORS  = ['black',        'green',       'red',        'orange',        'blue',         'darkviolet',      'teal'           ]
ALG_MARKERS = ['o',            's',           '^',          'D',             'v',             '*',              'p'              ]
GLOVE_D = 200
FASTTEXT_D = 300
SIFT_D = 128
EQUALIZED_D = 128
GLOVE_N = 400000
FASTTEXT_N = 2000000
SIFT_N = 11164866
EQUALIZED_N = 400000
THREAD_VALS = ["1", "2", "4", "8", "12", "16", DEFAULT_THREADS]


    
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

def get_multitest_run(
        dataset,
        device="host",
        k=K_FOR_SCATTER,
        results=results
    ):
    r = results["multitest"][str(device)][str(dataset)][str(k)]
    # Convert results to arrays
    for alg in ALG_RECALLS + ALG_TIMES:
        if alg in r:
            r[alg] = as_array(r[alg])
    return r

def per_query_time(times, num_queries, ms=False, mu_s=False):
    if ms:
        return (times / num_queries) * 1000
    if mu_s:
        return (times / num_queries) * 1000000
    return times / num_queries

def queries_per_second(times, num_queries, ms=False):
    if ms:
        return (num_queries / times) / 1000
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
        if xlim is True:
            plt.xlim(left=0)
        else:
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

def make_legend_lines(colors, alphas, styles, marks):
    # NOTE: Legend names beginning with "_" are excluded, so this function will not always be needed.
    return [Line2D([0], [0], color=c, alpha=a, lw=2, linestyle=s, marker=m) for c, a, s, m in zip(colors, alphas, styles, marks)]


#============= GRAPHS =========================================================#


#------------- Alg Recall vs. K -------------------#
rs   = get_kvaried_run("host", SIFT,     SIFT_D,      SIFT_N,      DEFAULT_THREADS)
rf   = get_kvaried_run("host", FASTTEXT, FASTTEXT_D,  FASTTEXT_N,  DEFAULT_THREADS)
rg   = get_kvaried_run("host", GLOVE,    GLOVE_D,     GLOVE_N,     DEFAULT_THREADS)
k  = rg["k_values"]

# Setup Custom Legend
legend_names = DATASET_NAMES + [""] + ALG_NAMES.copy()[1:]
legend_colors = ["black", "black", "black", "black"] + ALG_COLORS.copy()[1:]
# legend_alphas = [1, 0.5, 0.25, 0.0] + [1] * (len(ALG_NAMES)-1)
legend_alphas = [1, 1, 1, 0] + [1] * (len(ALG_NAMES)-1)
legend_styles = ['-', '--', ':', '-'] + ['-'] * (len(ALG_NAMES)-1)
legend_marks  = ['', '', '', ''] + ALG_MARKERS[1:]
legend_lines = make_legend_lines(legend_colors, legend_alphas, legend_styles, legend_marks)
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
    for alg, color, m in zip(ALG_RECALLS[1:], ALG_COLORS[1:], ALG_MARKERS[1:]):
        # try:
        plt.plot(k, dataset[alg], color=color, alpha=alpha, linestyle=mark, marker=m)
        # plt.plot([DEFAULT_K], eq[alg], color=color, alpha=alpha, linestyle=mark, marker=m, markersize=3)
        # except KeyError:
        #     pass
finish_plot("recall_vs_k.png")

# NOTES: These vary hugely by parameters, dataset size, dataset semantics


#------------- Alg Recall, Semantic Comparison (BAR) --------#
hatches = ["", "////", 'xxxx']
x = np.arange(len(ALG_NAMES)-1)  # the label locations
width = 0.25  # the width of the bars
fig, ax = plt.subplots(figsize=(6, 4))
plt.title(f"Recall Semantic Comparison\n(Device-Independent; k={DEFAULT_K})")
plt.grid(True, axis='y', alpha=0.6)
bars1 = ax.bar(x - width, [rs[alg][-1] for alg in ALG_RECALLS[1:]], width*.8, label='SIFT', color=ALG_COLORS[1:], hatch=hatches[0], edgecolor='white')
bars2 = ax.bar(x, [rf[alg][-1] for alg in ALG_RECALLS[1:]], width*.8, label='FastText', color=ALG_COLORS[1:], hatch=hatches[1], edgecolor='white')
bars3 = ax.bar(x + width, [rg[alg][-1] for alg in ALG_RECALLS[1:]], width*.8, label='GloVe', color=ALG_COLORS[1:], hatch=hatches[2], edgecolor='white')
ax.set_xticks(x)
ax.set_xticklabels(ALG_NAMES[1:])
ax.set_ylabel(f"{DEFAULT_K}-Recall@{DEFAULT_K}")
ax.set_ylim(0, 1)
plt.legend(loc='upper left')
finish_plot("recall_semantic.png")


#------------- Alg Recall, Semantic Comparison (Dataset-Size-Equalized) (BAR) --------#
rs_E = get_kvaried_run("host", SIFT,     EQUALIZED_D, EQUALIZED_N, DEFAULT_THREADS)
rf_E = get_kvaried_run("host", FASTTEXT, EQUALIZED_D, EQUALIZED_N, DEFAULT_THREADS)
rg_E = get_kvaried_run("host", GLOVE,    EQUALIZED_D, EQUALIZED_N, DEFAULT_THREADS)

fig, ax = plt.subplots(figsize=(6, 4))
plt.title(f"Recall Semantic Comparison on Equalized Datasets\n(Device-Independent; k={DEFAULT_K})")
plt.grid(True, axis='y', alpha=0.6)
bars1 = ax.bar(x - width, [rs_E[alg][-1] for alg in ALG_RECALLS[1:]], width*.8, label='SIFT', color=ALG_COLORS[1:], hatch=hatches[0], edgecolor='white')
bars2 = ax.bar(x, [rf_E[alg][-1] for alg in ALG_RECALLS[1:]], width*.8, label='FastText', color=ALG_COLORS[1:], hatch=hatches[1], edgecolor='white')
bars3 = ax.bar(x + width, [rg_E[alg][-1] for alg in ALG_RECALLS[1:]], width*.8, label='GloVe', color=ALG_COLORS[1:], hatch=hatches[2], edgecolor='white')
ax.set_xticks(x)
ax.set_xticklabels(ALG_NAMES[1:])
ax.set_ylabel(f"{DEFAULT_K}-Recall@{DEFAULT_K}")
ax.set_ylim(0, 1)
plt.legend(loc='upper left')
finish_plot("recall_semantic_equalized.png")


#------------- Alg Recall vs. K (Data Scaling) --------#
x = [100000, 200000, 500000, 1000000, 2000000, 5000000, 10000000]
x_labels = ["100k", "200k", "500k", "1M", "2M", "5M", "10M"]
rs_scale = [get_kvaried_run("host", SIFT, SIFT_D, n, DEFAULT_THREADS) for n in x]
for r in rs_scale:
    assert r["k_values"][0] == DEFAULT_K, f"Expected k={DEFAULT_K} for all runs"
for alg, color, name, m in zip(ALG_RECALLS[1:], ALG_COLORS[1:], ALG_NAMES[1:], ALG_MARKERS[1:]):
    plt.plot(x, [r[alg][0] for r in rs_scale], color=color, label=name, marker=m)
setup_plot(
    title=f"Algorithm Recall vs. Dataset Size\n(Device-Independent; k={DEFAULT_K})",
    xlabel="SIFT Subset Size", ylabel=f"{DEFAULT_K}-Recall@{DEFAULT_K}",
    xscale='log', ylim=True, legend=False,
)
plt.xticks(x, labels=x_labels)
plt.legend(loc='lower left')
finish_plot("recall_vs_dataset_size.png")


#------------- Time Cost of Recall (SCATTER) --------#
ms = get_multitest_run(SIFT)
mf = get_multitest_run(FASTTEXT)
mg = get_multitest_run(GLOVE)


# Add custom legend points
plt.scatter([], [], color='black', alpha=1, s=12, label='SIFT')
plt.scatter([], [], color='black', alpha=0.5, s=8, label='FastText')
plt.scatter([], [], color='black', alpha=0.2, s=4, label='GloVe')
plt.scatter([], [], alpha=0, label='') # Spacer
# Normal plotting
for dataset, alpha, size in ((ms, 1, 12), (mf, .5, 8), (mg, .2, 4)):
    for alg, times, color, m in zip(ALG_RECALLS, ALG_TIMES, ALG_COLORS, ALG_MARKERS):
        t = queries_per_second(dataset[times], dataset["test_size"], ms=False)
        plt.scatter(dataset[alg], t, color=color, alpha=alpha, s=size, marker=m)
setup_plot(
    title=f"Time Cost of Recall, Varying Parameters\n(Host; k={DEFAULT_K})",
    xlabel=f"{DEFAULT_K}-Recall@{DEFAULT_K}",
    ylabel="Queries / Sec.",
    yscale='log',
    legend=False,
)
# plt.ylim(bottom=1)
plt.legend(['SIFT', 'FastText', 'GloVe', ''] + ALG_NAMES, bbox_to_anchor=(1.35, 1))
# plt.legend(['SIFT', 'FastText', 'GloVe', ''] + [item for pair in zip(ALG_NAMES, ['_']*len(ALG_NAMES)) for item in pair], bbox_to_anchor=(1.35, 1))
finish_plot("scatter_time_cost.png")


#------------- Query Time vs. K (Per Dataset) --------#
rs_b  = get_kvaried_run("bf3", SIFT,     SIFT_D,      SIFT_N,      DEFAULT_THREADS)
rf_b  = get_kvaried_run("bf3", FASTTEXT, FASTTEXT_D,  FASTTEXT_N,  DEFAULT_THREADS)
rg_b  = get_kvaried_run("bf3", GLOVE,    GLOVE_D,     GLOVE_N,     DEFAULT_THREADS)
r_h = (rs, rf, rg)
r_b = (rs_b, rf_b, rg_b)

for _h, _b, _n in zip(r_h, r_b, DATASET_NAMES):
    # Full graph for each dataset
    plt.plot([], [], color='black', linestyle='-')
    plt.plot([], [], color='black', linestyle='--')
    plt.plot([], [], alpha=0, label='') # Spacer
    for dataset, style, dev in ((_h, "-", "Host"), (_b, "--", "BF3")):
        for times, color, m, name in zip(ALG_TIMES, ALG_COLORS, ALG_MARKERS, ALG_NAMES):
            t = queries_per_second(dataset[times], dataset["query_size"], ms=False)
            plt.plot(k, t, color=color, linestyle=style, marker=m, label=dev+" "+name)
    setup_plot(
        title=f"Query Speed vs. k ({_n})",
        xlabel="k",
        ylabel="Queries / Sec.",
        xscale="log", yscale="log",
        xticks=k,
        legend=False,
    )
    # plt.ylim(bottom=1)
    # Add custom legend points
    plt.legend(['Host', 'BF3', ''] + ALG_NAMES, bbox_to_anchor=(1.35, 1))
    finish_plot(f"speed_vs_k_{_n}.png")

# NOTES: k doesn't change the speed much. Better shown with next graph (combined, on k=10).


#------------- Query Time Device Comparison --------#
ki = list(k).index(DEFAULT_K)

# Custom Legend
plt.scatter([], [], color='black', marker='o', label="Host")
plt.scatter([], [], edgecolors='black', facecolors='none', marker='o', label='BF3')
plt.scatter([], [], color='black', alpha=0, label=' ')
for name, alpha, size in zip(DATASET_NAMES, (1.0, 0.5, 0.2), (30, 20, 10)):
    plt.scatter([], [], color='black', marker='o', label=name, s=size, alpha=alpha)
plt.scatter([], [], color='black', alpha=0, label=' ')

# Plot Chosen params
for size, alpha, host_data, bf3_data in ((30, 1.0, rs, rs_b), (20, .5, rf, rf_b), (10, .2, rg, rg_b)):
    for alg, times, color, m, name in zip(ALG_RECALLS, ALG_TIMES, ALG_COLORS, ALG_MARKERS, ALG_NAMES):
        try:
            t = queries_per_second(host_data[times], host_data["query_size"], ms=False)
            plt.scatter([host_data[alg][ki]], [t[ki]], alpha=alpha, color=color, s=size, marker=m, label=name if alpha==1.0 else "_")
        except KeyError: pass
        try:
            # Bluefield point
            tb = queries_per_second(bf3_data[times], bf3_data["query_size"], ms=False)
            plt.scatter([bf3_data[alg][ki]], [tb[ki]], alpha=alpha, edgecolors=color, facecolors='none', s=size, marker=m, label="_")
        except KeyError: pass
        try:
            # Connecting line
            plt.plot([host_data[alg][ki], bf3_data[alg][ki]], [t[ki], tb[ki]], color=color, alpha=alpha)
        except KeyError: pass
setup_plot(
    title=f"Query Time Device Comparison (k={DEFAULT_K})",
    xlabel=f"{DEFAULT_K}-Recall@{DEFAULT_K}",
    ylabel="Queries / Sec.",
    yscale='log',
    legend=False,
)
plt.legend(bbox_to_anchor=(1.35, 1))
finish_plot(f"time_comparison.png")


#------------- Query Time Device Comparison (Dataset Scaling) --------#
rs_b_scale = [get_kvaried_run("bf3", SIFT, SIFT_D, n, DEFAULT_THREADS) for n in x]
for r in rs_b_scale:
    assert r["k_values"][0] == DEFAULT_K, f"Expected k={DEFAULT_K} for all runs"
sizes = np.linspace(4, 50, len(x))
alphas = np.linspace(0.1, 1.0, len(x))

# plt.figure(figsize=(5, 3))

# Custom Legend
plt.scatter([], [], color='black', marker='o', label="Host")
plt.scatter([], [], edgecolors='black', facecolors='none', marker='o', label='BF3')
plt.scatter([], [], color='black', alpha=0, label=' ')
# for name, alpha, size in zip(x_labels, alphas, sizes):
#     plt.scatter([], [], color='black', marker='o', label=name, s=size, alpha=alpha)
# plt.scatter([], [], color='black', alpha=0, label=' ')

for host_data, bf3_data, size, alpha in zip(rs_scale, rs_b_scale, sizes, alphas):
    for alg, times, color, m, name in zip(ALG_RECALLS, ALG_TIMES, ALG_COLORS, ALG_MARKERS, ALG_NAMES):
        try:
            t = queries_per_second(host_data[times], host_data["query_size"], ms=False)
            plt.scatter([host_data[alg][0]], [t[0]], alpha=alpha, color=color, s=size, marker=m, label=name if alpha==1.0 else "_")
        except KeyError: pass
        try:
            # Bluefield point
            tb = queries_per_second(bf3_data[times], bf3_data["query_size"], ms=False)
            plt.scatter([bf3_data[alg][0]], [tb[0]], alpha=alpha, edgecolors=color, facecolors='none', s=size, marker=m, label="_")
        except KeyError: pass
        try:
            # Connecting line
            plt.plot([host_data[alg][0], bf3_data[alg][0]], [t[0], tb[0]], color=color, alpha=alpha)
        except KeyError: pass
setup_plot(
    title=f"Query Time Device Comparison (k={DEFAULT_K})\n(SIFT Dataset Scaling 100k-10M: Bolder is Larger)",
    xlabel=f"{DEFAULT_K}-Recall@{DEFAULT_K}",
    ylabel="Queries / Sec.",
    yscale='log',
    # legend=False,
    xlim=True,
)
# plt.legend(bbox_to_anchor=(1.35, 1))
finish_plot(f"time_comparison_scaling.png")

# NOTES:
# - the leftward trend is the same as the downward trend in recall_vs_dataset_size.png
# - device gap decreases for some algorithms as dataset size increases
# - Most algs decrease in throughput, but IVFPQ on BF3 is the exception 


#------------- Query Time Device Comparison (Dataset Scaling, Thread-Equalized) --------#
rs_scale_16 = [get_kvaried_run("host", SIFT, SIFT_D, n, "16") for n in x]
rs_b_scale_16 = [get_kvaried_run("bf3", SIFT, SIFT_D, n, "16") for n in x]
for r in rs_scale_16:
    assert r["k_values"][0] == DEFAULT_K, f"Expected k={DEFAULT_K} for all runs"
for r in rs_b_scale_16:
    assert r["k_values"][0] == DEFAULT_K, f"Expected k={DEFAULT_K} for all runs"

# plt.figure(figsize=(5, 3))

# Custom Legend
plt.scatter([], [], color='black', marker='o', label="Host")
plt.scatter([], [], edgecolors='black', facecolors='none', marker='o', label='BF3')
plt.scatter([], [], color='black', alpha=0, label=' ')
# for name, alpha, size in zip(x_labels, alphas, sizes):
#     plt.scatter([], [], color='black', marker='o', label=name, s=size, alpha=alpha)
# plt.scatter([], [], color='black', alpha=0, label=' ')

for host_data, bf3_data, size, alpha in zip(rs_scale_16, rs_b_scale_16, sizes, alphas):
    for alg, times, color, m, name in zip(ALG_RECALLS, ALG_TIMES, ALG_COLORS, ALG_MARKERS, ALG_NAMES):
        try:
            t = queries_per_second(host_data[times], host_data["query_size"], ms=False)
            plt.scatter([host_data[alg][0]], [t[0]], alpha=alpha, color=color, s=size, marker=m, label=name if alpha==1.0 else "_")
        except KeyError: pass
        try:
            # Bluefield point
            tb = queries_per_second(bf3_data[times], bf3_data["query_size"], ms=False)
            plt.scatter([bf3_data[alg][0]], [tb[0]], alpha=alpha, edgecolors=color, facecolors='none', s=size, marker=m, label="_")
        except KeyError: pass
        try:
            # Connecting line
            plt.plot([host_data[alg][0], bf3_data[alg][0]], [t[0], tb[0]], color=color, alpha=alpha)
        except KeyError: pass
setup_plot(
    title=f"Query Time Device Comparison, Equalized to 16 Threads\n(k={DEFAULT_K}, SIFT scaling)",
    xlabel=f"{DEFAULT_K}-Recall@{DEFAULT_K}",
    ylabel="Queries / Sec.",
    yscale='log',
    # legend=False,
    xlim=True,
)
# plt.legend(bbox_to_anchor=(1.35, 1))
finish_plot(f"time_comparison_scaling_eq16threads.png")

# NOTES:
# Very little difference


# #------------- Query Time vs. K (Dataset-Equalized, Per Dataset) --------#
# # DATA NOT PRESENT FOR MORE THAN k=10; covered by time_comparison graphs.
# rs_Eb = get_kvaried_run("bf3", SIFT,     EQUALIZED_D, EQUALIZED_N, DEFAULT_THREADS)
# rf_Eb = get_kvaried_run("bf3", FASTTEXT, EQUALIZED_D, EQUALIZED_N, DEFAULT_THREADS)
# rg_Eb = get_kvaried_run("bf3", GLOVE,    EQUALIZED_D, EQUALIZED_N, DEFAULT_THREADS)
# r_Eh = (rs_E, rf_E, rg_E)
# r_Eb = (rs_Eb, rf_Eb, rg_Eb)

# for _h, _b, _n in zip(r_Eh, r_Eb, DATASET_NAMES):
#     # Full graph for each dataset
#     plt.plot([], [], color='black', linestyle='-')
#     plt.plot([], [], color='black', linestyle='--')
#     plt.plot([], [], alpha=0, label='') # Spacer
#     for dataset, style, dev in ((_h, "-", "Host"), (_b, "--", "BF3")):
#         for times, color, m, name in zip(ALG_TIMES, ALG_COLORS, ALG_MARKERS, ALG_NAMES):
#             t = queries_per_second(dataset[times], dataset["query_size"], ms=False)
#             plt.plot(k, t, color=color, linestyle=style, marker=m, label=dev+" "+name)
#     setup_plot(
#         title=f"Query Speed vs. k (Equalized {_n})",
#         xlabel="k",
#         ylabel="Queries / Sec.",
#         xscale="log", yscale="log",
#         xticks=k,
#         legend=False,
#     )
#     # plt.ylim(bottom=1)
#     # Add custom legend points
#     plt.legend(['Host', 'BF3', ''] + ALG_NAMES, bbox_to_anchor=(1.35, 1))
#     finish_plot(f"speed_vs_k_equalized_{_n}.png")


# #------------- Query Time vs. K (16-Thread-Equalized, Per Dataset) --------#
# # DATA NOT PRESENT FOR MORE THAN k=10; covered by time_comparison graphs.


#------------- Query Time vs. Threads --------#
th = [get_kvaried_run("host", GLOVE, GLOVE_D, GLOVE_N, t) for t in THREAD_VALS]
th_b = [get_kvaried_run("bf3", GLOVE, GLOVE_D, GLOVE_N, t) for t in THREAD_VALS]
threads_host = [int(v) for v in THREAD_VALS[:-1] + ["48"]]
threads_bf3 = [int(v) for v in THREAD_VALS[:-1] + ["16"]]

# Extract time for k=10 in each alg of each thread run, for host and bf3
times = {alg:[] for alg in ALG_TIMES}
times_b = {alg:[] for alg in ALG_TIMES}
for r, rb in zip(th, th_b):
    for alg in ALG_TIMES:
        if len(r[alg]) == 1: times[alg].append(queries_per_second(r[alg][0], r["query_size"], ms=False))
        else: times[alg].append(queries_per_second(r[alg][ki], r["query_size"], ms=False))
        if len(rb[alg]) == 1: times_b[alg].append(queries_per_second(rb[alg][0], rb["query_size"], ms=False))
        else: times_b[alg].append(queries_per_second(rb[alg][ki], rb["query_size"], ms=False))

# Custom legend
plt.plot([], [], color='black', linestyle='-', label='Host')
plt.plot([], [], color='black', linestyle='--', label='BF3')
plt.plot([], [], color='black', alpha=0, label=' ')

# Main plot
for alg, color, m, name in zip(ALG_TIMES, ALG_COLORS, ALG_MARKERS, ALG_NAMES):
    plt.plot(threads_host, times[alg], label=name, color=color, marker=m, linestyle="-")
    plt.plot(threads_bf3[:-1], times_b[alg][:-1], label="_", color=color, marker=m, linestyle='--')
setup_plot(
    title="Thread Scaling (GloVe; k=10)",
    xlabel="Threads",
    ylabel="Queries / Sec.", 
    xscale='log', xticks=threads_host, 
    yscale='log',
)
finish_plot("thread_scaling.png")
    

#------------- Throughput vs. Dataset Size --------#
# use rs_scale and rs_b_scale
times = {alg:[] for alg in ALG_TIMES}
times_b = {alg:[] for alg in ALG_TIMES}
for r, rb in zip(rs_scale, rs_b_scale):
    for alg in ALG_TIMES:
        if len(r[alg]) == 1: times[alg].append(queries_per_second(r[alg][0], r["query_size"], ms=False))
        else: times[alg].append(queries_per_second(r[alg][ki], r["query_size"], ms=False))
        if len(rb[alg]) == 1: times_b[alg].append(queries_per_second(rb[alg][0], rb["query_size"], ms=False))
        else: times_b[alg].append(queries_per_second(rb[alg][ki], rb["query_size"], ms=False))

plt.plot([], [], color='black', linestyle='-', label='Host')
plt.plot([], [], color='black', linestyle='--', label='BF3')
plt.plot([], [], color='black', alpha=0, label=' ')

# Main plot
for alg, color, m, name in zip(ALG_TIMES, ALG_COLORS, ALG_MARKERS, ALG_NAMES):
    plt.plot(x, times[alg], label=name, color=color, marker=m, linestyle="-")
    plt.plot(x, times_b[alg], label="_", color=color, marker=m, linestyle='--')
setup_plot(
    title="Throughput vs. Dataset Size (SIFT; k=10)",
    xlabel="SIFT Subset Size",
    ylabel="Queries / Sec.", 
    xscale='log',
    yscale='log',
)
plt.xticks(x, labels=x_labels)
finish_plot("speed_data_scaling.png")


#------------- Throughput vs. Dataset Size (16-Thread-Equalized) --------#
# use rs_scale_16 and rs_b_scale_16
times = {alg:[] for alg in ALG_TIMES}
times_b = {alg:[] for alg in ALG_TIMES}
for r, rb in zip(rs_scale_16, rs_b_scale_16):
    for alg in ALG_TIMES:
        if len(r[alg]) == 1: times[alg].append(queries_per_second(r[alg][0], r["query_size"], ms=False))
        else: times[alg].append(queries_per_second(r[alg][ki], r["query_size"], ms=False))
        if len(rb[alg]) == 1: times_b[alg].append(queries_per_second(rb[alg][0], rb["query_size"], ms=False))
        else: times_b[alg].append(queries_per_second(rb[alg][ki], rb["query_size"], ms=False))

plt.plot([], [], color='black', linestyle='-', label='Host')
plt.plot([], [], color='black', linestyle='--', label='BF3')
plt.plot([], [], color='black', alpha=0, label=' ')

# Main plot
for alg, color, m, name in zip(ALG_TIMES, ALG_COLORS, ALG_MARKERS, ALG_NAMES):
    plt.plot(x, times[alg], label=name, color=color, marker=m, linestyle="-")
    plt.plot(x, times_b[alg], label="_", color=color, marker=m, linestyle='--')
setup_plot(
    title="Throughput vs. Dataset Size, Equalized to 16 Threads\n(SIFT; k=10)",
    xlabel="SIFT Subset Size",
    ylabel="Queries / Sec.", 
    xscale='log',
    yscale='log',
)
plt.xticks(x, labels=x_labels)
finish_plot("speed_data_scaling_eq16threads.png")


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

'''

GRAPHS NOTES (Not all ended up as described here, but it's a good approximation)

COLOR KEY:
- flat: black
- lsh: green (dark)
- pq: red
- ivfpq: goldish orange
- hnsw: blue (dark)
- hnsw_pq: purple
- hnsw_sq: teal (dark)
(these are darker colors because lighter will be used elsewhere)
(Note that all graphs should have a legend)

DEVICE KEY:
- Solid line: host
- dotted line: BF3

SYMBOL KEY:
- To make our graphs colorblind accessible, we may actually need to use other shapes of markers and such, later. Remind me after we nail down these graphs!


--------------------------------
Title: "Algorithm Recall vs. k\n(Device-Independent)"
Instructions:
      - This information is device-independent (algorithmic properties) so only graph the host lines.
      - three lines for each algorithm (7 algorithms * 3 indexes = 21 lines total)
      - Indexes to pull from: glove, fasttext, sift, where glove is graphed as 1 alpha, fasttext as .67 alpha, and sift as .33 alpha.
      - We may separate these into different graphs later, but for now I want to see how they all compare.
Type: Line graph (no markers)
Log scaling: x axis
x axis label: "k Neighbors"
y axis label: "k-Recall@k"
Background grid: light gray (or semitransparent), not dotted
axis range limits: force y to be [0, 1]

* ALSO: Make a version/iteration of this graph that pulls from glove_128d_400k, fasttext_128d_400k, and sift_128d_400k instead, to see how dataset affects it without respect to size. Title: "Algorithm Recall vs. k, Equalized Datasets/n(Device-Independent)"

* ALSO: Make a version of this graph which pulls from the various subsets of sift (using more alpha steps) instead of the three different datasets, to see how dataset size affects it. Though that's a lot of lines, so we might separate by algorithm in that case and make 7 different graphs. In this case, the title for each would be something like "HNSW-PQ Recall vs. k on Subsets of Sift (Device-Independent)"

--------------------------------
Title: "Time Cost of Recall with Varying Parameters"
Instructions:
      - "multitest" results in the json file
      - In each algorithm's entry, you will find two multidimensional arrays: times and recalls. Use the size of the query set for the given dataset (it varies for each) to calculate queries/seconds (should be num_queries/times, since I'm almost positive times is in seconds, but we can double-check). Flatten these arrays so that q/s is the y and recall is the x.
      - One set of colored points for each algorithm, per each dataset (so three sets per alg) (use the same dataset varying alpha as in the last graph)
      - Again, these may be later separated into different graphs, but for now we want to compare them.
      - Note that k is fixed: k=10
Type: Scatter Plot
Log scaling: Both axes
x axis label: "10-Recall@10"
y axis label: "Queries per Second (1/s)"
Background grid: horizontal lines only. Same gray as for other graphs.
axis range limits: Not sure yet; let's try forcing y to start at 0 (for clarity, otherwise it looks deceptive)

---------------------------------
Title: "Query Time vs. k ([Dataset_Name])"
Instructions:
      - Three of this graph, one for each dataset
      - This graph will have solid lines for each alg on the host, and dotted for each on the bluefield, with matching colors for easy comparison.
      - Not queries/sec for this one, just time
      - Careful how you label the legend, so that we can tell which lines are which. Also, the legends have to be hard-coded, so make sure the labels are in the correct order!
      - 14 lines (7 algorithms, on 2 devices)
      - For this one, we probably need to put the legend off to the side, since the lines will fill the space
Type: Line graph
Log scaling: x axis
x axis label: "k Neighbors"
y axis label: "Time (sec.) for [num_queries] Queries"
Background grid: Yes, horiz and vertical
axis ranges limits: y should be forced to start at 0

-----------------------------------
Title: "Query Time vs. k (Equalized to 16 Threads, [Dataset_Name])"
Instructions:
      - Same as above, but instead of pulling threads="default", use threads=16 for both host and bluefield.

-----------------------------------
Title: "Query Time vs. Threads, [Dataset_name] (k=10)"
Instructions:
      - Three of this graph, one for each dataset
      - Since the last two graphs are mostly flat lines, we can ignore k for future graphs, and use a fixed k=10 whenever we measure speed.
      - One line for each algorithm, for each device (14 lines)
      - Include host's threads="default" as final data point for host, which will extend beyond where the bf3 line ends, because it doesn't have that many threads. Host's default (or full thread count) is 48.
Type: Line graph
log scaling: both axes, I think
x axis label: "Threads"
y axis label: "Time (sec.) for [num_queries] Queries"
Background Grid: Yes
axis range limits: y should be forced to start at 0 I think

-----------------------------------
Title: "Throughput vs. Dataset Size (Sift, k=10)"
Instructions:
      - One line for each algorithm, for each device (14 lines)
      - Here we use queries/sec again! And since each subset of sift has a different number of query vectors, make sure you properly calculate num_queries/sec for each data point (can be done with numpy array math).
      - Using subsets of sift. Probably don't need to include actual "sift" index, just 100k-10m sets.
Type: Line graph
log scaling: both axes, I think
x axis label: "Num Vecs in Dataset"
y axis label: "Queries per Second per Thread (1/s)"
Background grid: Yes
axis range limits: y should be forced to start at 0 I think

* ALSO make a version/iteration of this graph using threads=16 instead of threads="default". Title: "Throughput vs. Dataset Size,\nEqualized to 16 Threads (Sift, k=10)"

------------------------------------
Title: "Throughput vs. Number of Queries (Sift, k=10)"
(We don't have this data yet - I forgot to implement this.)

'''