import matplotlib.pyplot as plt
import numpy as np


# DATA : All data comes from bf3host_log_k10_multi.txt, run on glove.6B.200d.txt
host_LSH_k10_recall   =   np.array([0.1814, 0.2716, 0.3422, 0.4014, 0.4389])
host_LSH_k10_time     =   np.array([0.09369638, 0.07801421, 0.10790243, 0.16929569, 0.24661489]) # All these are Search times, not build

host_PQ_k10_recall    =  np.array([0.1478, 0.2013, 0.2346, 0.3877, 0.4508, 0.6049,
                          0.2206, 0.2928, 0.3523, 0.5483, 0.6104, 0.7718])
host_PQ_k10_time      =  np.array([0.04935188, 0.0631966,  0.08354286, 0.13346617, 0.2258872,  0.25608645,
                          0.28311777, 0.45357781, 0.56533347, 1.10801453, 1.34134911, 2.37059066])

host_IVFPQ_k10_recall = np.array([0.1318, 0.2096, 0.2439, 0.3875, 0.4428, 0.604 ,
                                  0.1326, 0.2103, 0.2444, 0.3885, 0.4448, 0.6019,
                                  0.137,  0.2161, 0.248,  0.3918, 0.439,  0.6054,
                                  0.1407, 0.2216, 0.2549, 0.3917, 0.4513, 0.6114,
                                  0.1481, 0.2279, 0.2605, 0.3957, 0.451,  0.6068,
                                  0.1512, 0.2244, 0.2555, 0.388,  0.4396, 0.5823,
                                  0.1937, 0.3178, 0.3696, 0.5593, 0.6228, 0.771 ,
                                  0.1962, 0.3275, 0.3771, 0.5557, 0.6199, 0.7744,
                                  0.1992, 0.3111, 0.3584, 0.5489, 0.6228, 0.7719,
                                  0.2029, 0.3118, 0.3577, 0.556,  0.6164, 0.7707,
                                  0.2031, 0.3123, 0.3662, 0.548,  0.6081, 0.7591,
                                  0.2058, 0.3197, 0.3557, 0.5253, 0.5978, 0.7249])
host_IVFPQ_k10_time   = np.array([0.05645957, 0.06282316, 0.12887639, 0.13357753, 0.18787236, 0.26135121,
                                  0.05581867, 0.06276742, 0.12734022, 0.13166853, 0.18626516, 0.26474065,
                                  0.05321883, 0.05936221, 0.12136302, 0.12444651, 0.17543829, 0.24981729,
                                  0.03967796, 0.0445958,  0.08797821, 0.0931378,  0.13095679, 0.18460435,
                                  0.03087261, 0.03473735, 0.06838999, 0.07319922, 0.10356212, 0.14516545,
                                  0.01884667, 0.02106053, 0.04103,    0.04425134, 0.06042092, 0.08578111,
                                  0.29340505, 0.53759353, 0.66087242, 1.31016324, 1.60060616, 2.64152055,
                                  0.2936734,  0.537353,   0.65940813, 1.30188058, 1.63428097, 2.63212109,
                                  0.27960513, 0.50757042, 0.62711092, 1.23178991, 1.50458103, 2.47118232,
                                  0.20828176, 0.38131106, 0.4683668,  0.99884254, 1.125868,   1.78082262,
                                  0.1627288,  0.29789343, 0.36957798, 0.72049397, 0.86976058, 1.41725447,
                                  0.0982186,  0.17936766, 0.22088947, 0.42851132, 0.51496834, 0.84599951])

# NOTE: This data was done with a test size of 1000; thus we could simply use these numbers as miliseconds instead of seconds per thousand;
#   HOWEVER, instead we will use queries per second (1000 / times)

# GRAPHS :
plt.title("Time Cost of Recall Across Methods (Host; varying parameters)")
plt.grid(color='0.8')
# plt.scatter(host_IVFPQ_k10_recall, host_IVFPQ_k10_time, color='orange')
# plt.scatter(host_PQ_k10_recall, host_PQ_k10_time, color='teal')
# plt.scatter(host_LSH_k10_recall, host_LSH_k10_time, color='purple')
# plt.ylabel("Time (ms)")
plt.scatter(host_IVFPQ_k10_recall, 1000/host_IVFPQ_k10_time, color='orange')
plt.scatter(host_PQ_k10_recall, 1000/host_PQ_k10_time, color='teal')
plt.scatter(host_LSH_k10_recall, 1000/host_LSH_k10_time, color='purple')
plt.ylabel("Queries per Second (1/s)")
plt.xlabel("10-Recall@10")
plt.xlim(0, 1.1)
plt.legend(["IVFPQ", "PQ", "LSH"])
# plt.show()#block=False)
plt.savefig("results/recall_vs_speed.png")
plt.close()

# Logarithmic Graph:
plt.title("Time Cost of Recall Across Methods (Host; varying parameters)")
plt.grid(color='0.8')
plt.loglog(host_IVFPQ_k10_recall, 1000/host_IVFPQ_k10_time, 'o', color='orange')
plt.loglog(host_PQ_k10_recall, 1000/host_PQ_k10_time, 'o', color='teal')
plt.loglog(host_LSH_k10_recall, 1000/host_LSH_k10_time, 'o', color='purple')
plt.ylabel("Queries per Second (1/s)")
plt.xlabel("10-Recall@10")
# plt.xlim(0, 1.1)
plt.legend(["IVFPQ", "PQ", "LSH"])
# plt.show()#block=False)
plt.savefig("results/recall_vs_speed_logarithmic.png")
plt.close()




# # Modified from https://github.com/erikbern/ann-benchmarks/blob/main/ann_benchmarks/plotting/utils.py
# def create_pointset(recall, time):
#     data = 
#     data.sort(key=lambda t: (-1*t[-1], -1*t[-2]))

#     axs, ays, als = [], [], []
#     # Generate Pareto frontier
#     xs, ys, ls = [], [], []
#     last_x = float("-inf")
#     comparator = (lambda xv, lx: xv > lx) if last_x < 0 else (lambda xv, lx: xv < lx)
#     xv, yv = data
#     axs.append(xv)
#     ays.append(yv)
#     if comparator(xv, last_x):
#         last_x = xv
#         xs.append(xv)
#         ys.append(yv)
#     return xs, ys, ls, axs, ays, als