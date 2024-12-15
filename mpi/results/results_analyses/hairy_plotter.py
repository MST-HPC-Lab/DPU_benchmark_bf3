# Make plots for benchmark data

import matplotlib.pyplot as plt

# X axes for Strong Scaling
num_procs           = [2,  4,  8,  16] # BF3 wouldn't do 16 with MPI, for some reason.
num_partitions      = [64, 64, 64, 64]
num_partitions_weak = [8,  16, 32, 64]

# Time Benchmarks
time_seqH  = [52.9317,  53.1069,  65.5753,  69.932 ] # Sequential on Host (Varies due to increased overhead I presume; we just left extra processes idle)
time_rrH   = [39.3298,  21.6023,  22.0495,  21.7889] # Round Robin ''
time_rr1H  = [56.619 ,  40.0195,  28.5321,  23.2107] # Round Robin with one less worker ''
time_lbH   = [52.9092,  24.0418,  20.123 ,  11.1171] # Load-Balancing ''
time_seqBF = [88.6066,  None   ,  88.6151,  None   ] # '' on BlueField-3 (Same variation reasons as above)
time_rrBF  = [62.5302,  None   ,  29.9113,  None   ] # '' on BlueField-3
time_rr1BF = [88.4665,  None   ,  37.6748,  None   ] # '' on BlueField-3
time_lbBF  = [89.6512,  None   ,  26.5081,  None   ] # '' on BlueField-3
time_labels = ["Host Seq. Time", "Host RR Time", "Host RR-1 Time", "Host LB Time",
               "BF3 Seq. Time",  "BF3 RR Time",  "BF3 RR-1 Time",  "BF3 LB Time"]

# plt.title("Time Benchmark")
# plt.plot(num_procs, time_seqH , 'r-', linewidth=2) # label=time_labels[0]
# plt.plot(num_procs, time_rrH  , 'm-')
# plt.plot(num_procs, time_rr1H , 'm:')
# plt.plot(num_procs, time_lbH  , 'r-')
# plt.plot(num_procs, time_seqBF, 'b-', linewidth=2)
# plt.plot(num_procs, time_rrBF , 'c-')
# plt.plot(num_procs, time_rr1BF, 'c:')
# plt.plot(num_procs, time_lbBF , 'b-')
# # plt.set_xticklabels(num_procs)
# plt.xlabel("Number of Processes")
# plt.ylabel("Time (Sec.)")
# plt.legend(time_labels)
# plt.show()

# Speedup
title_speedup = "Speedup"
speedup_rrH   = [1.34584,  2.45839,  2.974  ,  3.20952]
speedup_rr1H  = [0.93487,  1.32702,  2.2983 ,  3.01292]
speedup_lbH   = [1.00042,  2.20894,  3.25872,  6.29049]
speedup_rrBF  = [1.41702,  None   ,  2.9626 ,  None   ]
speedup_rr1BF = [1.00158,  None   ,  2.3521 ,  None   ]
speedup_lbBF  = [0.98834,  None   ,  3.34295,  None   ]
speedup_labels = ["Host RR Speedup", "Host RR-1 Speedup", "Host LB Speedup",
                  "BF3 RR Speedup",  "BF3 RR-1 Speedup",  "BF3 LB Speedup"]

# Bottleneck Speedup (LB relative to RR)
title_bn = "Bottleneck Speedup (LB relative to RR)"
speedup_bnH   = [52.9092,  24.0418,  20.123,   11.1171]
speedup_bnBF  = [89.6512,  None   ,  26.5081,  None   ]
speedup_bn_labels = ["Host", "BF3"]

# Efficiency
title_efficiency = "Efficiency"
efficiency_seq   = [1,        1,        1,        1      ]
efficiency_rrH   = [0.67292,  0.61459,  0.37175,  0.20059]
efficiency_rr1H  = [0.93487,  0.44234,  0.32832,  0.20086]
efficiency_lbH   = [0.50021,  0.55223,  0.40734,  0.39315]
efficiency_rrBF  = [0.70851,  None   ,  0.37032,  None   ]
efficiency_rr1BF = [1.00158,  None   ,  0.33601,  None   ]
efficiency_lbBF  = [0.49417,  None   ,  0.41786,  None   ]

# Compute Cost
title_cost = "Compute Cost"
cost_seqH  = [52.9317,  53.1069,  65.5753,  69.932 ]
cost_rrH   = [78.6595,  86.4093,  176.396,  348.623]
cost_rr1H  = [56.619 ,  120.059,  199.724,  348.161]
cost_lbH   = [105.818,  96.1672,  160.984,  177.873]
cost_seqBF = [88.6066,  None   ,  88.6151,  None   ]
cost_rrBF  = [125.06 ,  None   ,  239.29 ,  None   ]
cost_rr1BF = [88.4665,  None   ,  263.724,  None   ]
cost_lbBF  = [179.302,  None   ,  212.064,  None   ]

# Strong Scalability Bar Graph
title_strong_scalability = "Strong Scalability (Quantitative Estimate)"
strong_scalability_rrH  = speedup_rrH[2]  / speedup_rrH[0]
strong_scalability_lbH  = speedup_lbH[2]  / speedup_lbH[0]
strong_scalability_rrBF = speedup_rrBF[2] / speedup_rrBF[0]
strong_scalability_lbBF = speedup_lbBF[2] / speedup_lbBF[0]
baseline = 1

# Weak Scaling Time
title_weak_time = "Time as CPUs & Problem Size Increase (Weak-Scaling)"
weak_time_seqH  = [0.01412,  1.87034,  5.01933,  69.932 ]
weak_time_rrH   = [0.01344,  1.00371,  1.79677,  21.7889]
weak_time_rr1H  = [0.01240,  1.26431,  2.03035,  23.2107]
weak_time_lbH   = [0.02636,  0.68547,  1.43295,  11.1171]
weak_time_seqBF = [0.02781,  3.38366,  7.25505,  None   ]
weak_time_rrBF  = [0.02629,  1.72191,  0.27992,  None   ]
weak_time_rr1BF = [0.02690,  2.23364,  1.34656,  None   ]
weak_time_lbBF  = [0.02770,  1.73843,  2.08419,  None   ]
weak_axis = [f"{a}\n{b}" for (a, b) in zip((num_procs, num_partitions_weak))]
weak_axis_label = "Processes, Partitions used"

# Weak Scaling Speedup
title_weak_speedup = "Speedup as CPUs & Problem Size Increase (Weak-Scaling)"
weak_speedup_rrH   = [1.05063,  1.86343,  2.79353,  3.20952]
weak_speedup_rr1H  = [1.13881,  1.47933,  2.47215,  3.01292]
weak_speedup_lbH   = [0.53581,  2.72854,  3.5028 ,  6.29049]
weak_speedup_rrBF  = [1.05789,  1.96507,  25.9177,  None   ]
weak_speedup_rr1BF = [1.03382,  1.51487,  5.38785,  None   ]
weak_speedup_lbBF  = [1.00409,  1.94639,  3.481  ,  None   ]

# Weak Scalability Bar Graph
title_weak_scalability = "Weak Scalability (Quantitative Estimate)"
weak_scalability_rrH  = weak_speedup_rrH[2]  / weak_speedup_rrH[0]
weak_scalability_lbH  = weak_speedup_lbH[2]  / weak_speedup_lbH[0]
weak_scalability_rrBF = weak_speedup_rrBF[2] / weak_speedup_rrBF[0]
weak_scalability_lbBF = weak_speedup_lbBF[2] / weak_speedup_lbBF[0]