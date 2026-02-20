# Make plots for benchmark data

import matplotlib.pyplot as plt

# X axes for Strong Scaling
num_procs           = [2,  4,  8,  16] # BF3 wouldn't do 16 with MPI, for some reason.
num_procs_bf3       = [2, 8]
num_partitions      = [64, 64, 64, 64]
num_partitions_weak = [8,  16, 32, 64]

#-------------------------------------------------------------------------------

# Time Benchmarks
title_time = "Time Benchmark"
time_seqH  = [52.9317,  53.1069,  65.5753,  69.932 ] # Sequential on Host (Varies due to increased overhead I presume; we just left extra processes idle)
time_rrH   = [39.3298,  21.6023,  22.0495,  21.7889] # Round Robin ''
# time_rr1H  = [56.619 ,  40.0195,  28.5321,  23.2107] # Round Robin with one less worker ''
time_lbH   = [52.9092,  24.0418,  20.123 ,  11.1171] # Load-Balancing ''
# time_seqBF = [88.6066,  None   ,  88.6151,  None   ] # '' on BlueField-3 (Same variation reasons as above)
# time_rrBF  = [62.5302,  None   ,  29.9113,  None   ] # '' on BlueField-3
# # time_rr1BF = [88.4665,  None   ,  37.6748,  None   ] # '' on BlueField-3
# time_lbBF  = [89.6512,  None   ,  26.5081,  None   ] # '' on BlueField-3
time_seqBF = [88.6066,  88.6151] # '' on BlueField-3 (Same variation reasons as above)
time_rrBF  = [62.5302,  29.9113] # '' on BlueField-3
# time_rr1BF = [88.4665,  37.6748] # '' on BlueField-3
time_lbBF  = [89.6512,  26.5081] # '' on BlueField-3
# time_labels = ["Host Seq. Time", "Host RR Time", "Host RR-1 Time", "Host LB Time",
            #    "BF3 Seq. Time",  "BF3 RR Time",  "BF3 RR-1 Time",  "BF3 LB Time"]
time_labels = ["Host Seq. Time", "Host RR Time", "Host LB Time",
               "BF3 Seq. Time",  "BF3 RR Time",  "BF3 LB Time"]

# plt.figure()
plt.title(title_time)
plt.plot(num_procs,     time_seqH , color='orange', linewidth=3) # label=time_labels[0]
plt.plot(num_procs,     time_rrH  , 'm-')
# plt.plot(num_procs,     time_rr1H , 'm:')
plt.plot(num_procs,     time_lbH  , 'r-')
plt.plot(num_procs_bf3, time_seqBF, 'b-', linewidth=3)
plt.plot(num_procs_bf3, time_rrBF , 'c-')
# plt.plot(num_procs_bf3, time_rr1BF, 'c:')
plt.plot(num_procs_bf3, time_lbBF , color='steelblue')
# plt.set_xticklabels(num_procs)
plt.ylim(bottom=0)
plt.xlabel("Number of Processes")
plt.ylabel("Time (Sec.)")
plt.legend(time_labels)
# plt.show()#block=False)
plt.savefig(title_time + ".png")
plt.close()

#-------------------------------------------------------------------------------

# Speedup
title_speedup = "Speedup Benchmark"
speedup_rrH   = [1.34584,  2.45839,  2.974  ,  3.20952]
# speedup_rr1H  = [0.93487,  1.32702,  2.2983 ,  3.01292]
speedup_lbH   = [1.00042,  2.20894,  3.25872,  6.29049]
# speedup_rrBF  = [1.41702,  None   ,  2.9626 ,  None   ]
# # speedup_rr1BF = [1.00158,  None   ,  2.3521 ,  None   ]
# speedup_lbBF  = [0.98834,  None   ,  3.34295,  None   ]
speedup_rrBF  = [1.41702,  2.9626 ]
# speedup_rr1BF = [1.00158,  2.3521 ]
speedup_lbBF  = [0.98834,  3.34295]
# speedup_labels = ["Host RR Speedup", "Host RR-1 Speedup", "Host LB Speedup",
#                   "BF3 RR Speedup",  "BF3 RR-1 Speedup",  "BF3 LB Speedup"]
speedup_labels = ["Host RR Speedup", "Host LB Speedup",
                  "BF3 RR Speedup",  "BF3 LB Speedup"]

# plt.figure()
plt.title(title_speedup)
plt.plot(num_procs,     speedup_rrH  , 'm-')
# plt.plot(num_procs,     speedup_rr1H , 'm:')
plt.plot(num_procs,     speedup_lbH  , 'r-')
plt.plot(num_procs_bf3, speedup_rrBF , 'c-')
# plt.plot(num_procs_bf3, speedup_rr1BF, 'c:')
plt.plot(num_procs_bf3, speedup_lbBF , color='steelblue')
plt.ylim(bottom=0)
# plt.set_xticklabels(num_procs)
plt.xlabel("Number of Processes")
plt.ylabel("Speedup (seq_t / par_t)")
plt.legend(speedup_labels)
# plt.show()#block=False)
plt.savefig(title_speedup + ".png")
plt.close()

#-------------------------------------------------------------------------------

# Bottleneck Speedup (LB relative to RR)
title_bn = "Bottleneck Speedup (Load-Balancing vs. Round-Robin)"
speedup_bnH   = [52.9092,  24.0418,  20.123,   11.1171]
# speedup_bnBF  = [89.6512,  None   ,  26.5081,  None   ]
speedup_bnBF  = [89.6512, 26.5081]
speedup_bn_labels = ["Host", "BF3"]

# plt.figure()
plt.title(title_bn)
plt.plot(num_procs,     speedup_bnH , color='fuchsia')
plt.plot(num_procs_bf3, speedup_bnBF , color='darkviolet')
# plt.set_xticklabels(num_procs)
# plt.ylim(0, 1)
plt.ylim(bottom=0)
plt.xlabel("Number of Processes")
plt.ylabel("Speedup")
plt.legend(speedup_bn_labels)
# plt.show()#block=False)
plt.savefig(title_bn + ".png")
plt.close()

#-------------------------------------------------------------------------------

# Efficiency
title_efficiency = "Efficiency"
efficiency_seq   = [1,        1,        1,        1      ]
efficiency_rrH   = [0.67292,  0.61459,  0.37175,  0.20059]
# efficiency_rr1H  = [0.93487,  0.44234,  0.32832,  0.20086]
efficiency_lbH   = [0.50021,  0.55223,  0.40734,  0.39315]
# efficiency_rrBF  = [0.70851,  None   ,  0.37032,  None   ]
# # efficiency_rr1BF = [1.00158,  None   ,  0.33601,  None   ]
# efficiency_lbBF  = [0.49417,  None   ,  0.41786,  None   ]
efficiency_rrBF  = [0.70851,  0.37032]
# efficiency_rr1BF = [1.00158,  0.33601]
efficiency_lbBF  = [0.49417,  0.41786]
# efficiency_labels = ["Seq. Effic.", "Host RR Effic.", "Host RR-1 Effic.", "Host LB Effic.",
#                "BF3 RR Effic.",  "BF3 RR-1 Effic.",  "BF3 LB Effic."]
efficiency_labels = ["Seq. Effic.", "Host RR Effic.", "Host LB Effic.",
                                    "BF3 RR Effic.",  "BF3 LB Effic."]

# plt.figure()
plt.title(title_efficiency)
plt.plot(num_procs,     efficiency_seq , color='orange', linewidth=3) # label=time_labels[0]
plt.plot(num_procs,     efficiency_rrH  , 'm-')
# plt.plot(num_procs,     efficiency_rr1H , 'm:')
plt.plot(num_procs,     efficiency_lbH  , 'r-')
plt.plot(num_procs_bf3, efficiency_rrBF , 'c-')
# plt.plot(num_procs_bf3, efficiency_rr1BF, 'c:')
plt.plot(num_procs_bf3, efficiency_lbBF , color='steelblue')
# plt.set_xticklabels(num_procs)
plt.ylim(0, 1.1)
plt.xlabel("Number of Processes")
plt.ylabel("Efficiency")
plt.legend(efficiency_labels)
# plt.show()#block=False)
plt.savefig(title_efficiency + ".png")
plt.close()

#-------------------------------------------------------------------------------

# Compute Cost
title_cost = "Compute Cost (equivalent to seq. time)"
cost_seqH  = [52.9317,  53.1069,  65.5753,  69.932 ]
cost_rrH   = [78.6595,  86.4093,  176.396,  348.623]
# cost_rr1H  = [56.619 ,  120.059,  199.724,  348.161]
cost_lbH   = [105.818,  96.1672,  160.984,  177.873]
# cost_seqBF = [88.6066,  None   ,  88.6151,  None   ]
# cost_rrBF  = [125.06 ,  None   ,  239.29 ,  None   ]
# # cost_rr1BF = [88.4665,  None   ,  263.724,  None   ]
# cost_lbBF  = [179.302,  None   ,  212.064,  None   ]
cost_seqBF = [88.6066,  88.6151]
cost_rrBF  = [125.06 ,  239.29 ]
# cost_rr1BF = [88.4665,  263.724]
cost_lbBF  = [179.302,  212.064]
# labels = ["Host Seq.", "Host RR", "Host RR-1", "Host LB",
#                "BF3 Seq.",  "BF3 RR",  "BF3 RR-1",  "BF3 LB"]
labels = ["Host Seq.", "Host RR", "Host LB",
          "BF3 Seq.",  "BF3 RR",  "BF3 LB"]

# plt.figure()
plt.title(title_cost)
plt.plot(num_procs,     cost_seqH , color='orange', linewidth=3) # label=time_labels[0]
plt.plot(num_procs,     cost_rrH  , 'm-')
# plt.plot(num_procs,     cost_rr1H , 'm:')
plt.plot(num_procs,     cost_lbH  , 'r-')
plt.plot(num_procs_bf3, cost_seqBF, 'b-', linewidth=3)
plt.plot(num_procs_bf3, cost_rrBF , 'c-')
# plt.plot(num_procs_bf3, cost_rr1BF, 'c:')
plt.plot(num_procs_bf3, cost_lbBF , color='steelblue')
# plt.set_xticklabels(num_procs)
plt.ylim(bottom=0)
plt.xlabel("Number of Processes")
plt.ylabel("Seq. Time Equivalent Cost")
plt.legend(labels)
# plt.show()#block=False)
plt.savefig(title_cost + ".png")
plt.close()

#-------------------------------------------------------------------------------

# Strong Scalability Bar Graph
title_strong_scalability = "Strong Scalability (Quantitative Estimate)"
strong_scalability_rrH  = speedup_rrH[2]  / speedup_rrH[0]
strong_scalability_lbH  = speedup_lbH[2]  / speedup_lbH[0]
strong_scalability_rrBF = speedup_rrBF[1] / speedup_rrBF[0]
strong_scalability_lbBF = speedup_lbBF[1] / speedup_lbBF[0]
# baseline = 1
scalability_categories = ["Host Round-Robin", "Host Load-Bal.", "BF3 Round-Robin", "BF3 Load-Bal."]
scalability_colors = ['m', 'r', 'c', 'steelblue']

# plt.figure()
plt.title(title_strong_scalability)
plt.bar(scalability_categories, [strong_scalability_rrH, strong_scalability_lbH, strong_scalability_rrBF, strong_scalability_lbBF], color=scalability_colors)
plt.axhline(y=1, color="k")
# plt.ylim(bottom=0)
# plt.xlabel("Category")
plt.ylabel("Strong Scalability")
# plt.show()#block=False)
plt.savefig(title_strong_scalability + ".png")
plt.close()

#-------------------------------------------------------------------------------

num_procs_short = [2,  4,  8]

# Weak Scaling Time
title_weak_time = "Time as CPUs & Problem Size Increase (Weak-Scaling)"
weak_time_seqH  = [0.01412,  1.87034,  5.01933,  69.932 ]
weak_time_rrH   = [0.01344,  1.00371,  1.79677,  21.7889]
# weak_time_rr1H  = [0.01240,  1.26431,  2.03035,  23.2107]
weak_time_lbH   = [0.02636,  0.68547,  1.43295,  11.1171]
weak_time_seqBF = [0.02781,  3.38366,  7.25505]#,  None   ]
weak_time_rrBF  = [0.02629,  1.72191,  0.27992]#,  None   ]
# weak_time_rr1BF = [0.02690,  2.23364,  1.34656]#,  None   ]
weak_time_lbBF  = [0.02770,  1.73843,  2.08419]#,  None   ]
weak_axis = [f"{a}\n{b}" for (a, b) in zip(num_procs, num_partitions_weak)]
weak_axis_label = "Processes, Partitions used"

# plt.figure()
plt.title(title_weak_time)
plt.plot(num_procs,     weak_time_seqH , color='orange', linewidth=3) # label=time_labels[0]
plt.plot(num_procs,     weak_time_rrH  , 'm-')
# plt.plot(num_procs,     weak_time_rr1H , 'm:')
plt.plot(num_procs,     weak_time_lbH  , 'r-')
plt.plot(num_procs[:-1], weak_time_seqBF, 'b-', linewidth=3)
plt.plot(num_procs[:-1], weak_time_rrBF , 'c-')
# plt.plot(num_procs_bf3, weak_time_rr1BF, 'c:')
plt.plot(num_procs[:-1], weak_time_lbBF , color='steelblue')
# plt.set_xticklabels(num_procs)
plt.ylim(bottom=0)
plt.xlabel(weak_axis_label)
plt.ylabel("Time (Sec.)")
plt.legend(labels)
# plt.show()#block=False)
plt.savefig(title_weak_time + ".png")
plt.close()

#-------------------------------------------------------------------------------

# Weak Scaling Speedup
title_weak_speedup = "Speedup as CPUs & Problem Size Increase (Weak-Scaling)"
weak_speedup_rrH   = [1.05063,  1.86343,  2.79353,  3.20952]
# weak_speedup_rr1H  = [1.13881,  1.47933,  2.47215,  3.01292]
weak_speedup_lbH   = [0.53581,  2.72854,  3.5028 ,  6.29049]
weak_speedup_rrBF  = [1.05789,  1.96507,  25.9177]#,  None   ]
# weak_speedup_rr1BF = [1.03382,  1.51487,  5.38785,  None   ]
weak_speedup_lbBF  = [1.00409,  1.94639,  3.481  ]#,  None   ]

# plt.figure()
plt.title(title_weak_speedup)
plt.plot(num_procs,     weak_speedup_rrH  , 'm-')
# plt.plot(num_procs,     weak_speedup_rr1H , 'm:')
plt.plot(num_procs,     weak_speedup_lbH  , 'r-')
plt.plot(num_procs[:-1], weak_speedup_rrBF , 'c-')
# plt.plot(num_procs[:-1], weak_speedup_rr1BF, 'c:')
plt.plot(num_procs[:-1], weak_speedup_lbBF , color='steelblue')
# plt.set_xticklabels(num_procs)
plt.ylim(bottom=0)
plt.xlabel(weak_axis_label)
plt.ylabel("Speedup")
plt.legend(scalability_categories)
# plt.show()#block=False)
plt.savefig(title_weak_speedup + ".png")
plt.close()

#-------------------------------------------------------------------------------

# Weak Scalability Bar Graph
title_weak_scalability = "Weak Scalability (Quantitative Estimate)"
weak_scalability_rrH  = weak_speedup_rrH[2]  / weak_speedup_rrH[0]
weak_scalability_lbH  = weak_speedup_lbH[2]  / weak_speedup_lbH[0]
weak_scalability_rrBF = weak_speedup_rrBF[2] / weak_speedup_rrBF[0]
weak_scalability_lbBF = weak_speedup_lbBF[2] / weak_speedup_lbBF[0]

# plt.figure()
plt.title(title_weak_scalability)
plt.bar(scalability_categories, [weak_scalability_rrH, weak_scalability_lbH, weak_scalability_rrBF, weak_scalability_lbBF], color=scalability_colors)
plt.axhline(y=1, color="k")
# plt.ylim(bottom=0)
# plt.xlabel("Category")
plt.ylabel("Weak Scalability")
# plt.show()#block=False)
plt.savefig(title_weak_scalability + ".png")
plt.close()

#-------------------------------------------------------------------------------