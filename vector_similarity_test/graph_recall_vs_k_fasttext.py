import matplotlib.pyplot as plt
import numpy as np
import matplotlib

def apply_legend(outside=True, ncol=1):
    if outside:
        plt.legend(
            loc='lower right',
            bbox_to_anchor=(5, 0),
            frameon=True,
            fancybox=True,
            framealpha=1,
            ncol=ncol
        )
        plt.tight_layout(rect=[0, 0, 0.8, 1])
    else:
        plt.legend(
            loc='best',
            frameon=True,
            fancybox=True,
            framealpha=1,
            ncol=ncol
        )

matplotlib.rcParams['font.family'] = 'DejaVu Serif'

matplotlib.rcParams['font.size'] = 12
matplotlib.rcParams['axes.titlesize'] = 14
matplotlib.rcParams['axes.labelsize'] = 12
matplotlib.rcParams['legend.fontsize'] = 7

matplotlib.rcParams['xtick.labelsize'] = 11
matplotlib.rcParams['ytick.labelsize'] = 11
matplotlib.rcParams['axes.linewidth'] = 1.2
matplotlib.rcParams['legend.frameon'] = True

# DATA :
k = np.array([1, 2, 3, 5, 7, 10, 25, 50, 75, 100])

#host recalls
#fast_host_lsh_recall = np.array()
fast_host_lsh_recall = np.array([1.0, 0.72655, 0.6345666666666673, 0.5534599999999912, 0.5155571428571503, 0.4802300000000015, 0.42108399999998963, 0.3920419999999964, 0.3783359999999954, 0.3699970000000008])
#fast_host_lsh_recall = np.array([1.0, 0.72655, 0.6345666666666673, 0.5534599999999912, 0.5155571428571503, 0.4802300000000015, 0.42108399999998963, 0.3920419999999964, 0.3783359999999954, 0.3699970000000008])

#fast_host_pq_recall = np.array()
fast_host_pq_recall = np.array([0.9991, 0.7023, 0.6035666666666645, 0.5245599999999903, 0.48750000000000976, 0.46010000000000184, 0.42311199999999455, 0.4143560000000054, 0.4135013333333403, 0.41469299999999987])
#fast_host_pq_recall = np.array([0.9991, 0.7023, 0.6035666666666645, 0.5245599999999903, 0.48750000000000976, 0.46010000000000184, 0.42311199999999455, 0.4143560000000054, 0.4135013333333403, 0.41469299999999987])

#fast_host_ivfpq_recall = np.array()
fast_host_ivfpq_recall = np.array([0.9992, 0.7006, 0.6053333333333297, 0.5235599999999879, 0.4875285714285802, 0.4645700000000026, 0.4237999999999938, 0.41434200000000554, 0.4130960000000059, 0.41328399999999865])
#fast_host_ivfpq_recall = np.array([0.9992, 0.7006, 0.6053333333333297, 0.5235599999999879, 0.4875285714285802, 0.4645700000000026, 0.4237999999999938, 0.41434200000000554, 0.4130960000000059, 0.41328399999999865])

#fast_host_hnsw_recall = np.array()
fast_host_hnsw_recall = np.array([0.8413, 0.8266, 0.8227000000000071, 0.8220600000000144, 0.8214000000000024, 0.8201799999999877, 0.8097320000000309, 0.7902659999999749, 0.7719173333333743, 0.7542799999999841])
#fast_host_hnsw_recall = np.array([0.8941, 0.87445, 0.8683333333333371, 0.8661600000000117, 0.8639142857142877, 0.8613199999999859, 0.8493520000000242, 0.8319759999999684, 0.8153000000000449, 0.7993919999999807])

#host times
#fast_host_bf_time = np.array()
fast_host_bf_time = np.array([415.30410946079644, 725.8568133655994, 362.08520634440646, 364.3155802477995, 366.0469445399998, 360.9939645375998, 354.8305135824019, 372.63218768119697, 366.060916124197, 364.6083379755961])
#fast_host_bf_time = np.array([208.38042158974955, 352.3189749137188, 362.4151365061601, 368.47042726073414, 362.67198857214925, 385.9652954802538, 378.7179944979337, 371.67291618697345, 363.18255674187094, 359.6219363611502])

#fast_host_lsh_time = np.array()
fast_host_lsh_time = np.array([109.40501456779893, 47.380592053601866, 47.13518486439425, 47.72836112719669, 47.164362930195054, 47.248798508592884, 47.580645404593085, 47.708677489403634, 47.75851977620623, 48.23002664739033])
#fast_host_lsh_time = np.array([46.86785144436484, 47.62866039512058, 46.99359579430893, 47.060058168601245, 47.077925475003816, 48.550081433262676, 48.282890192233026, 47.88132502123093, 47.860778062293925, 47.88160128612071])

#fast_host_pq_time = np.array()
fast_host_pq_time = np.array([24.379611080998437, 12.57539002780104, 12.517412413406419, 12.574065466196043, 12.518692972196732, 12.543796037603169, 12.539697211398742, 12.647399263002445, 12.619208412594162, 12.640417255001376])
#fast_host_pq_time = np.array([12.540313832927495, 12.633938771672547, 12.525809552054852, 12.588103749323636, 12.541989596250156, 12.527978024290254, 12.553150044288486, 12.625101076283803, 12.841607447558394, 12.671807037045559])

#fast_host_ivfpq_time = np.array()
fast_host_ivfpq_time = np.array([48.3759638004005, 21.664323843395685, 21.671527010199497, 21.770357155401143, 21.786763464199613, 21.84128216060344, 21.96254613180063, 21.756430483591977, 22.60022314780508, 22.419333752209788])
#fast_host_ivfpq_time = np.array([21.945247192246217, 22.102694308695693, 21.90119551618894, 21.640325841028243, 21.96166699162374, 22.009793200995773, 22.110285801813006, 22.168253949688125, 23.08678574487567, 22.888718753742676])

#fast_host_hnsw_time = np.array()
fast_host_hnsw_time = np.array([1.6590944042021873, 1.2676485492003848, 1.204874912800733, 1.142412122595124, 1.1245483637962024, 1.0883555498032365, 1.0972624720074236, 1.0402881483954842, 1.026167135796277, 0.9867385896039196])
#fast_host_hnsw_time = np.array([0.6516187850696346, 0.6535738993746539, 0.6617486020550132, 0.6533154362502197, 0.6456360194521645, 0.6454550533865889, 0.6515327667196592, 0.6516772884254655, 0.6508354656398296, 0.6551335520731906])


# bf3_log_kvaried.txt
#bf3 
fast_bf3_lsh_recall = np.array([1.0, 0.72655, 0.6345666666666673, 0.5534599999999912, 0.5155571428571503, 0.4802300000000015, 0.42108399999998963, 0.3920419999999964, 0.3783359999999954, 0.3699970000000008])
#fast_bf3_lsh_recall = np.array([1.0, 0.8, 0.7399999999999999, 0.6739999999999996, 0.6257142857142857, 0.5890000000000001, 0.496, 0.4443999999999999, 0.4184000000000001, 0.40160000000000023])
#fast_bf3_lsh_recall = np.array([1.0, 0.7115, 0.6076666666666675, 0.5105999999999982, 0.4688571428571441, 0.4389000000000008, 0.38772, 0.36773999999999996, 0.36329333333333375, 0.3618700000000005])
# bf3_lsh_recall = np.array([1.0, 0.66525, 0.53666666666667, 0.42550000000000526, 0.3760000000000029, 0.34230000000000244, 0.2774200000000012, 0.2546600000000007, 0.24898000000000034, 0.24770000000000042])

fast_bf3_pq_recall = np.array([0.9991, 0.7023, 0.6035666666666645, 0.5245599999999903, 0.48750000000000976, 0.46010000000000184, 0.42311199999999455, 0.4143560000000054, 0.4135026666666737, 0.41469299999999987])
#fast_bf3_pq_recall = np.array([1.0, 0.81, 0.7533333333333332, 0.7219999999999998, 0.6942857142857138, 0.672, 0.6187999999999998, 0.608, 0.6106666666666667, 0.6128])
#fast_bf3_pq_recall = np.array([1.0, 0.7585, 0.6973333333333316, 0.6378000000000013, 0.6119999999999985, 0.6049000000000018, 0.5972399999999993, 0.60748, 0.615893333333333, 0.6225699999999994])
# bf3_pq_recall = np.array([1.0, 0.6605, 0.5486666666666677, 0.45800000000000207, 0.42407142857143265, 0.39314999999999894, 0.3644800000000016, 0.36591000000000096, 0.37101333333333214, 0.376675])

fast_bf3_ivfpq_recall = np.array([0.9992, 0.7006, 0.6053333333333297, 0.5235599999999879, 0.4875285714285802, 0.4645700000000026, 0.4237999999999938, 0.41434200000000554, 0.41309733333333926, 0.41328399999999865])
#fast_bf3_ivfpq_recall = np.array([1.0, 0.84, 0.7533333333333334, 0.7139999999999996, 0.6914285714285711, 0.6750000000000004, 0.6232, 0.6125999999999999, 0.6062666666666668, 0.6132])
#fast_bf3_ivfpq_recall = np.array([1.0, 0.7635, 0.6999999999999984, 0.6370000000000009, 0.6139999999999975, 0.6040000000000021, 0.6081599999999993, 0.6212000000000011, 0.6316533333333326, 0.6374999999999997])
# bf3_ivfpq_recall = np.array([1.0, 0.66775, 0.5556666666666683, 0.4641000000000019, 0.42685714285714754, 0.3998499999999986, 0.36948000000000136, 0.3682999999999999, 0.37278000000000033, 0.37814500000000073])

fast_bf3_hnsw_recall = np.array([0.9375, 0.91865, 0.9146000000000019, 0.9137000000000107, 0.9140714285714383, 0.9139099999999774, 0.9119840000000131, 0.9055159999999549, 0.8993053333333433, 0.893101999999967])
#fast_bf3_hnsw_recall = np.array([0.97, 0.965, 0.97, 0.954, 0.9542857142857142, 0.9519999999999998, 0.9479999999999997, 0.9353999999999997, 0.9253333333333335, 0.9150000000000003])
#fast_bf3_hnsw_recall = np.array([0.546, 0.6515, 0.7059999999999985, 0.7533999999999973, 0.7778571428571445, 0.7967999999999963, 0.8254, 0.832640000000002, 0.8252400000000004, 0.8148300000000002])

#bf times 
fast_bf3_bf_time = np.array([57.346970305824655, 53.565637098811564, 53.17351095220074, 54.77803040179424, 53.71203052101191, 54.16182984580519, 54.056472794199365, 53.743999425985386, 53.241793731006325, 53.5589075845899])
#fast_bf3_bf_time = np.array([0.8942022820313772, 1.2510318520168464, 1.0483403280377388, 0.975078321993351, 1.2236991475025814, 1.0211786382521193, 1.132761608498792, 1.1799139141415556, 0.9897462371736765, 1.1863993328685563])
#fast_bf3_bf_time = np.array([1.3506282413145527, 1.4998857273021713, 1.2808819750013452, 1.631202310983402, 1.6438660923546802, 1.7193028163552906, 1.7257996192978073, 1.7120429956897472, 1.6780553677429755, 1.846966523017424])
# bf3_bf_time = np.array([2.22197065067788, 2.2999817120532193, 1.929354372356708, 2.0866508673255644, 2.2116392282769084, 2.2885235116506615, 2.4379541659727693, 1.9571980152589579, 2.422278366750106, 2.417781783733517])

fast_bf3_lsh_time = np.array([44.373012008785736, 44.36846696479479, 44.3306740875938, 44.50601122799562, 44.42594538138947, 44.50003522561165, 44.760216734209095, 44.81998617418576, 44.98296429839684, 44.32675622858805])
#fast_bf3_lsh_time = np.array([1.003027295305704, 1.0199226920182507, 1.0066567906566586, 1.0394406153354794, 0.9876575509939963, 0.9824721637026718, 1.0115679770242423, 0.9813595673379799, 0.9539247600284094, 0.9164566240118196])
# bf3_lsh_time = np.array([0.6333923690641919, 0.6358298274377981, 0.6251845882895092, 0.710280463565141, 0.6406674272535989, 0.6176036049922308, 0.6372681430075318, 0.6193267253693193, 0.6307041998952627, 0.6268240106292069])

fast_bf3_pq_time = np.array([27.42813415400451, 27.419113428809215, 27.428267145785505, 27.507539343787357, 27.368324394198133, 27.370596827799453, 27.51340381698683, 27.490167166606987, 27.575330794986805, 27.557945214991925])
#fast_bf3_pq_time = np.array([0.6038905083357046, 0.602470847700412, 0.5995026153201858, 0.6131012233672664, 0.6042122679840153, 0.6098243963594238, 0.6201812940028807, 0.6068789726511264, 0.6104442293290049, 0.6049251819883162])
# bf3_pq_time = np.array([0.48704401768433553, 0.48690356966108084, 0.47544437274336815, 0.47760209441185, 0.47779803730857867, 0.4797634342685342, 0.4848640324392666, 0.5977221486003449, 0.49259017656246823, 0.5019057185854763])

fast_bf3_ivfpq_time = np.array([40.53455850977916, 40.42655186200282, 40.51563961219508, 40.523889870382845, 40.57065944019705, 40.60581860300154, 40.66859369140584, 40.65354626821354, 40.49789840738522, 40.626834766403775])
#fast_bf3_ivfpq_time = np.array([1.0706029509892687, 1.0799509283388034, 1.1118466136589025, 1.0715527013720323, 1.0590918780459713, 1.0725786396457504, 1.069908314035274, 1.0609194433394198, 1.0861408209893852, 1.12598904230011])
# bf3_ivfpq_time = np.array([0.9218621273369839, 0.9166570162245383, 0.9181425563680629, 0.936582489637658, 0.9212781870737672, 0.9206849056451271, 0.9281905816557506, 0.9435631791129708, 0.9320845709492763, 0.9522352425847203])

fast_bf3_hnsw_time = np.array([2.807705438591074, 2.8088270340114834, 2.826354219799396, 2.8209169360110535, 2.8079102820018305, 2.8328169758198785, 2.829438058391679, 2.8176921000122093, 2.824750755005516, 2.8484027674072423])
#fast_bf3_hnsw_time = np.array([0.46688861896594364, 0.46842890535481274, 0.46704568271525204, 0.4688688893026362, 0.4663217500007401, 0.468654470013765, 0.46898208364533883, 0.4687837020416434, 0.4684814266317214, 0.47032098231526714])  



# GRAPHS :
#1. Recall vs. K (Host vs. BF3)
plt.figure(figsize=(6,4))
plt.title("Recall vs. k (FastText Dataset, Host vs. BF3)")
#plt.title("fasttext cc.en.300.vec: How Recall Varies with k\n(other parameters chosen so that recall is 100% when k=1)")
plt.grid(True, linestyle='--', alpha=0.6)

#Host (Solid Lines)
plt.plot(k, fast_host_lsh_recall, color='purple', label='Host LSH', linewidth=2)
plt.plot(k, fast_host_pq_recall, color='teal', label='Host PQ', linewidth=2)
plt.plot(k, fast_host_ivfpq_recall, color='orange', label='Host IVFPQ', linewidth=2)
plt.plot(k, fast_host_hnsw_recall, color='navy', label='Host HNSW', linewidth=2)

#BF3 (Dotted lines with markers)
plt.plot(k, fast_bf3_lsh_recall, 'o:', color='purple', label='BF3 LSH', linewidth=2)
plt.plot(k, fast_bf3_pq_recall, 'o:', color='teal', label='BF3 PQ', linewidth=2)
plt.plot(k, fast_bf3_ivfpq_recall, 'o:', color='orange', label='BF3 IVFPQ', linewidth=2)
plt.plot(k, fast_bf3_hnsw_recall, 'o:', color='navy', label='BF3 HNSW', linewidth=2)

#plt.plot(k, fast_host_lsh_recall   ,       color='purple')
#plt.plot(k, fast_host_pq_recall    ,       color='teal')
#plt.plot(k, fast_host_ivfpq_recall ,       color='orange')
#plt.plot(k, fast_host_hnsw_recall ,        color ='navy')
# plt.plot(k, bf3_lsh_recall    , 'o:', color='purple')
# plt.plot(k, bf3_pq_recall     , 'o:', color='teal')
# plt.plot(k, bf3_ivfpq_recall  , 'o:', color='orange')

plt.xscale('log') 
plt.ylabel("Recall@k")
plt.xlabel("k") 
plt.ylim(0, 1.1) 
apply_legend()
#plt.legend(["LSH", "PQ", "IVFPQ", "HNSW"]) #["Host LSH", "Host PQ", "Host IVFPQ", "BF3 LSH", "BF3 PQ", "BF3 IVFPQ"]
# plt.show()#block=False)
#plt.savefig("results/fasttext_recall_vs_k.png")
plt.savefig("fasttext_recall_vs_k_host_vs_bf3.png", dpi=300, bbox_inches='tight')
plt.close()


#2. Time vs. K (Host vs. BF3)
plt.figure(figsize=(6,4))
#plt.title("fasttext cc.en.300.vec: How Recall Time Varies with k\n(other parameters chosen so that recall is 100% when k=1)")
plt.title("Time vs. k (FastText Dataset, Host vs. BF3)")
plt.grid(True, linestyle='--', alpha=0.6)

#Host (Solid Lines)
plt.plot(k, fast_host_bf_time, color='black', linestyle='--', alpha=0.6, label='Host Flat')
plt.plot(k, fast_host_lsh_time, color='purple', label='Host LSH', linewidth=2)
plt.plot(k, fast_host_pq_time, color='teal', label='Host PQ', linewidth=2)
plt.plot(k, fast_host_ivfpq_time, color='orange', label='Host IVFPQ', linewidth=2)
plt.plot(k, fast_host_hnsw_time, color='navy', label='Host HNSW', linewidth=2)

#BF3 (Dotted lines with markers)
plt.plot(k, fast_bf3_bf_time, 'o:', color='black', alpha=0.6, label='BF3 Flat')
plt.plot(k, fast_bf3_lsh_time, 'o:', color='purple', label='BF3 LSH', linewidth=2)
plt.plot(k, fast_bf3_pq_time, 'o:', color='teal', label='BF3 PQ', linewidth=2)
plt.plot(k, fast_bf3_ivfpq_time, 'o:', color='orange', label='BF3 IVFPQ', linewidth=2)
plt.plot(k, fast_bf3_hnsw_time, 'o:', color='navy', label='BF3 HNSW', linewidth=2)

# plot lines without markers to match the recall plot's simple style
#plt.plot(k, fast_host_bf_time    , color='k',    linewidth=1.0)
#plt.plot(k, fast_host_lsh_time   , color='purple',linewidth=1.0)
#plt.plot(k, fast_host_pq_time    , color='teal', linewidth=1.0)
#plt.plot(k, fast_host_ivfpq_time , color='orange',linewidth=1.0)
#plt.plot(k, fast_host_hnsw_time  , color='navy', linewidth=1.0)
#plt.plot(k, fast_bf3_bf_time     , 'o:', color='k')
#plt.plot(k, fast_bf3_lsh_time    , 'o:', color='purple')
#plt.plot(k, fast_bf3_pq_time     , 'o:', color='teal')
#plt.plot(k, fast_bf3_ivfpq_time  , 'o:', color='orange')
#plt.plot(k, fast_bf3_hnsw_time   , 'o:', color='navy')

plt.xscale('log')
plt.yscale('log')
plt.ylabel("Time (sec)")
plt.xlabel("k")
apply_legend(ncol=2)
plt.ylim(0.3, max(fast_host_bf_time)*1.2)
#plt.legend(["Host Flat","Host LSH","Host PQ","Host IVFPQ","Host HNSW","BF3 Flat","BF3 LSH","BF3 PQ","BF3 IVFPQ","BF3 HNSW"], fontsize='small')
plt.tight_layout()
plt.savefig("fasttext_time_vs_k_host_vs_bf3.png", dpi=300, bbox_inches='tight')
#plt.savefig("results/fasttext_recalltime_vs_k.png", dpi=300)
plt.close()


#3. Recall vs. Time (Host) — SCATTER
plt.figure(figsize=(6,4))
plt.title("Recall vs Time (FastText Dataset, Host)")
plt.grid(True, linestyle='--', alpha=0.6)

plt.scatter(fast_host_lsh_time, fast_host_lsh_recall, color='purple', label='LSH', s=60)
plt.scatter(fast_host_pq_time, fast_host_pq_recall, color='teal', label='PQ', s=60)
plt.scatter(fast_host_ivfpq_time, fast_host_ivfpq_recall, color='orange', label='IVFPQ', s=60)
plt.scatter(fast_host_hnsw_time, fast_host_hnsw_recall, color='navy', label='HNSW', s=60)

all_times = np.concatenate([fast_host_lsh_time, fast_host_pq_time, fast_host_ivfpq_time, fast_host_hnsw_time])
plt.xlim(all_times.min()*0.9, all_times.max()*1.1)
plt.ylim(0.35, 1.01)
plt.xlabel("Time (sec)")
plt.ylabel("Recall")
apply_legend()
plt.savefig("fasttext_recall_vs_time_host_scatter.png", dpi=300, bbox_inches='tight')
plt.close()


#4. Recall vs. Time (BF3) — SCATTER
plt.figure(figsize=(6,4))
plt.title("Recall vs Time (FastText Dataset, BF3)")
plt.grid(True, linestyle='--', alpha=0.6)

plt.scatter(fast_bf3_lsh_time, fast_bf3_lsh_recall, color='purple', label='LSH', s=60)
plt.scatter(fast_bf3_pq_time, fast_bf3_pq_recall, color='teal', label='PQ', s=60)
plt.scatter(fast_bf3_ivfpq_time, fast_bf3_ivfpq_recall, color='orange', label='IVFPQ', s=60)
plt.scatter(fast_bf3_hnsw_time, fast_bf3_hnsw_recall, color='navy', label='HNSW', s=60)

all_times = np.concatenate([fast_bf3_lsh_time, fast_bf3_pq_time, fast_bf3_ivfpq_time, fast_bf3_hnsw_time])
plt.xlim(all_times.min()*0.9, all_times.max()*1.1)
plt.ylim(0.35, 1.01)
plt.xlabel("Time (sec)")
plt.ylabel("Recall")
apply_legend()
plt.savefig("fasttext_recall_vs_time_bf3_scatter.png", dpi=300, bbox_inches='tight')
plt.close()