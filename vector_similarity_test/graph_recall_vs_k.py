import matplotlib.pyplot as plt
import numpy as np



# DATA :
k = np.array([1, 2, 3, 5, 7, 10, 25, 50, 75, 100])


# bf3host_log_kvaried.txt
host_lsh_recall = np.array([1.0, 0.66525, 0.5366666666666666, 0.4255, 0.376, 0.3423, 0.27742, 0.25465, 0.24897333333333332, 0.24769999999999998])
host_pq_recall = np.array([1.0, 0.65725, 0.5488333333333333, 0.45990000000000003, 0.423, 0.39265, 0.3648, 0.36661, 0.37173333333333336, 0.37726499999999996])
host_ivfpq_recall = np.array([1.0, 0.668, 0.5555, 0.4642, 0.4278571428571428, 0.399, 0.36962, 0.36863, 0.37261333333333335, 0.377935])

host_bf_time = np.array([0.2965616383589804, 0.31058788128818077, 0.32350668287836015, 0.2925656752971311, 0.29953429102897644, 0.29337027699997026, 0.2989283353090286, 0.29735877419201034, 0.30707130837254226, 0.34449729113839567])
host_lsh_time = np.array([0.2076359895678858, 0.22565887433787188, 0.20901780078808466, 0.20939074930114052, 0.21404668292962015, 0.21049358105907837, 0.24663752441604933, 0.2118577272631228, 0.21826705965213478, 0.21481160167604685])
host_pq_time = np.array([0.29653845929230255, 0.27195904271987575, 0.26737358335716027, 0.27586024968574446, 0.26767540560103953, 0.2707055943707625, 0.26879650362146396, 0.2741040310356766, 0.2722231193135182, 0.2772485096938908])
host_ivfpq_time = np.array([0.2298863271716982, 0.2294015733835598, 0.22945111834754547, 0.22925534805593392, 0.23243345770364007, 0.22921400905276337, 0.23300679967117807, 0.23103179899044335, 0.23166145027304688, 0.23564033373259008])


# bf3_log_kvaried.txt
bf3_lsh_recall = np.array([1.0, 0.66525, 0.53666666666667, 0.42550000000000526, 0.3760000000000029, 0.34230000000000244, 0.2774200000000012, 0.2546600000000007, 0.24898000000000034, 0.24770000000000042])
bf3_pq_recall = np.array([1.0, 0.6605, 0.5486666666666677, 0.45800000000000207, 0.42407142857143265, 0.39314999999999894, 0.3644800000000016, 0.36591000000000096, 0.37101333333333214, 0.376675])
bf3_ivfpq_recall = np.array([1.0, 0.66775, 0.5556666666666683, 0.4641000000000019, 0.42685714285714754, 0.3998499999999986, 0.36948000000000136, 0.3682999999999999, 0.37278000000000033, 0.37814500000000073])

bf3_bf_time = np.array([2.22197065067788, 2.2999817120532193, 1.929354372356708, 2.0866508673255644, 2.2116392282769084, 2.2885235116506615, 2.4379541659727693, 1.9571980152589579, 2.422278366750106, 2.417781783733517])
bf3_lsh_time = np.array([0.6333923690641919, 0.6358298274377981, 0.6251845882895092, 0.710280463565141, 0.6406674272535989, 0.6176036049922308, 0.6372681430075318, 0.6193267253693193, 0.6307041998952627, 0.6268240106292069])
bf3_pq_time = np.array([0.48704401768433553, 0.48690356966108084, 0.47544437274336815, 0.47760209441185, 0.47779803730857867, 0.4797634342685342, 0.4848640324392666, 0.5977221486003449, 0.49259017656246823, 0.5019057185854763])
bf3_ivfpq_time = np.array([0.9218621273369839, 0.9166570162245383, 0.9181425563680629, 0.936582489637658, 0.9212781870737672, 0.9206849056451271, 0.9281905816557506, 0.9435631791129708, 0.9320845709492763, 0.9522352425847203])



# GRAPHS :
plt.title("glove.6B.200d.txt: How Recall Varies with k\n(other parameters chosen so that recall is 100% when k=1)")
plt.grid(color='0.8')
plt.plot(k, host_lsh_recall   ,       color='purple')
plt.plot(k, host_pq_recall    ,       color='teal')
plt.plot(k, host_ivfpq_recall ,       color='orange')
# plt.plot(k, bf3_lsh_recall    , 'o:', color='purple')
# plt.plot(k, bf3_pq_recall     , 'o:', color='teal')
# plt.plot(k, bf3_ivfpq_recall  , 'o:', color='orange')
plt.xscale('log')
plt.ylabel("k-Recall@k")
plt.xlabel("k")
plt.ylim(0, 1.1)
plt.legend(["LSH", "PQ", "IVFPQ"]) #["Host LSH", "Host PQ", "Host IVFPQ", "BF3 LSH", "BF3 PQ", "BF3 IVFPQ"]
# plt.show()#block=False)
plt.savefig("results/recall_vs_k.png")
plt.close()


plt.title("glove.6B.200d.txt: How Recall Time Varies with k\n(other parameters chosen so that recall is 100% when k=1)")
plt.grid(color='0.8')
# plt.scatter(host_IVFPQ_k10_recall, host_IVFPQ_k10_time, color='orange')
# plt.scatter(host_PQ_k10_recall, host_PQ_k10_time, color='teal')
# plt.scatter(host_LSH_k10_recall, host_LSH_k10_time, color='purple')
# plt.ylabel("Time (ms)")
plt.plot(k, host_bf_time    ,       color='k')
plt.plot(k, host_lsh_time   ,       color='purple')
plt.plot(k, host_pq_time    ,       color='teal')
plt.plot(k, host_ivfpq_time ,       color='orange')
plt.plot(k, bf3_bf_time     , 'o:', color='k')
plt.plot(k, bf3_lsh_time    , 'o:', color='purple')
plt.plot(k, bf3_pq_time     , 'o:', color='teal')
plt.plot(k, bf3_ivfpq_time  , 'o:', color='orange')
plt.xscale('log')
plt.ylabel("Time (sec)")
plt.xlabel("k")
plt.ylim(bottom=0)
plt.legend(["Host Flat", "Host LSH", "Host PQ", "Host IVFPQ", "BF3 Flat", "BF3 LSH", "BF3 PQ", "BF3 IVFPQ"])
# plt.show()#block=False)
plt.savefig("results/recalltime_vs_k.png")
plt.close()