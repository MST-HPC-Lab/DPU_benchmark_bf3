if grep -q "BlueField" /sys/class/dmi/id/product_name; then
    echo "DETECTED BLUEFIELD ENVIRONMENT"
    
    # BF3 BUILD
    # python3 index_builder.py --file glove.6B.200d.txt --out_folder glove >> results/glove/bf3_build.txt 2>>&1 & disown
    # python3 index_builder.py --file cc.en.300.vec --out_folder fasttext >> results/fasttext/bf3_build.txt 2>>&1 & disown

    # BF3 SEARCH
    #### Glove
    python3 index_searcher.py --only "flat"  --threads 16 --indices_dir indices/glove_clean >> results/glove/bf3_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 16 --indices_dir indices/glove_clean >> results/glove/bf3_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 16 --indices_dir indices/glove_clean >> results/glove/bf3_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 16 --indices_dir indices/glove_clean >> results/glove/bf3_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 16 --indices_dir indices/glove_clean >> results/glove/bf3_hnsw.txt  2>>&1

    python3 index_searcher.py --only "flat"  --threads 8  --indices_dir indices/glove_clean >> results/glove/bf3_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 8  --indices_dir indices/glove_clean >> results/glove/bf3_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 8  --indices_dir indices/glove_clean >> results/glove/bf3_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 8  --indices_dir indices/glove_clean >> results/glove/bf3_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 8  --indices_dir indices/glove_clean >> results/glove/bf3_hnsw.txt  2>>&1
    
    python3 index_searcher.py --only "flat"  --threads 4  --indices_dir indices/glove_clean >> results/glove/bf3_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 4  --indices_dir indices/glove_clean >> results/glove/bf3_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 4  --indices_dir indices/glove_clean >> results/glove/bf3_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 4  --indices_dir indices/glove_clean >> results/glove/bf3_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 4  --indices_dir indices/glove_clean >> results/glove/bf3_hnsw.txt  2>>&1
    
    python3 index_searcher.py --only "flat"  --threads 2  --indices_dir indices/glove_clean >> results/glove/bf3_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 2  --indices_dir indices/glove_clean >> results/glove/bf3_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 2  --indices_dir indices/glove_clean >> results/glove/bf3_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 2  --indices_dir indices/glove_clean >> results/glove/bf3_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 2  --indices_dir indices/glove_clean >> results/glove/bf3_hnsw.txt  2>>&1
    
    python3 index_searcher.py --only "flat"  --threads 1  --indices_dir indices/glove_clean >> results/glove/bf3_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 1  --indices_dir indices/glove_clean >> results/glove/bf3_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 1  --indices_dir indices/glove_clean >> results/glove/bf3_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 1  --indices_dir indices/glove_clean >> results/glove/bf3_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 1  --indices_dir indices/glove_clean >> results/glove/bf3_hnsw.txt  2>>&1

    #### Fasttext
    python3 index_searcher.py --only "flat"  --threads 16 --indices_dir indices/fasttext_final >> results/fasttext/bf3_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 16 --indices_dir indices/fasttext_final >> results/fasttext/bf3_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 16 --indices_dir indices/fasttext_final >> results/fasttext/bf3_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 16 --indices_dir indices/fasttext_final >> results/fasttext/bf3_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 16 --indices_dir indices/fasttext_final >> results/fasttext/bf3_hnsw.txt  2>>&1

    python3 index_searcher.py --only "flat"  --threads 8  --indices_dir indices/fasttext_final >> results/fasttext/bf3_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 8  --indices_dir indices/fasttext_final >> results/fasttext/bf3_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 8  --indices_dir indices/fasttext_final >> results/fasttext/bf3_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 8  --indices_dir indices/fasttext_final >> results/fasttext/bf3_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 8  --indices_dir indices/fasttext_final >> results/fasttext/bf3_hnsw.txt  2>>&1
    
    python3 index_searcher.py --only "flat"  --threads 4  --indices_dir indices/fasttext_final >> results/fasttext/bf3_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 4  --indices_dir indices/fasttext_final >> results/fasttext/bf3_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 4  --indices_dir indices/fasttext_final >> results/fasttext/bf3_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 4  --indices_dir indices/fasttext_final >> results/fasttext/bf3_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 4  --indices_dir indices/fasttext_final >> results/fasttext/bf3_hnsw.txt  2>>&1
    
    python3 index_searcher.py --only "flat"  --threads 2  --indices_dir indices/fasttext_final >> results/fasttext/bf3_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 2  --indices_dir indices/fasttext_final >> results/fasttext/bf3_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 2  --indices_dir indices/fasttext_final >> results/fasttext/bf3_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 2  --indices_dir indices/fasttext_final >> results/fasttext/bf3_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 2  --indices_dir indices/fasttext_final >> results/fasttext/bf3_hnsw.txt  2>>&1
    
    python3 index_searcher.py --only "flat"  --threads 1  --indices_dir indices/fasttext_final >> results/fasttext/bf3_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 1  --indices_dir indices/fasttext_final >> results/fasttext/bf3_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 1  --indices_dir indices/fasttext_final >> results/fasttext/bf3_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 1  --indices_dir indices/fasttext_final >> results/fasttext/bf3_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 1  --indices_dir indices/fasttext_final >> results/fasttext/bf3_hnsw.txt  2>>&1

else
    echo "DETECTED HOST ENVIRONMENT"
    
    # Host BUILD
    # python3 index_builder.py --file glove.6B.200d.txt --out_folder glove >> results/glove/bf3host_build.txt 2>>&1 & disown
    # python3 index_builder.py --file cc.en.300.vec --out_folder fasttext >> results/fasttext/bf3host_build.txt 2>>&1 & disown

    # Host SEARCH (NOT disowned)
    #### Glove
    python3 index_searcher.py --only "flat"  --threads 16 --indices_dir indices/glove_clean >> results/glove/bf3host_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 16 --indices_dir indices/glove_clean >> results/glove/bf3host_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 16 --indices_dir indices/glove_clean >> results/glove/bf3host_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 16 --indices_dir indices/glove_clean >> results/glove/bf3host_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 16 --indices_dir indices/glove_clean >> results/glove/bf3host_hnsw.txt  2>>&1

    python3 index_searcher.py --only "flat"  --threads 8  --indices_dir indices/glove_clean >> results/glove/bf3host_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 8  --indices_dir indices/glove_clean >> results/glove/bf3host_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 8  --indices_dir indices/glove_clean >> results/glove/bf3host_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 8  --indices_dir indices/glove_clean >> results/glove/bf3host_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 8  --indices_dir indices/glove_clean >> results/glove/bf3host_hnsw.txt  2>>&1 

    python3 index_searcher.py --only "flat"  --threads 4  --indices_dir indices/glove_clean >> results/glove/bf3host_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 4  --indices_dir indices/glove_clean >> results/glove/bf3host_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 4  --indices_dir indices/glove_clean >> results/glove/bf3host_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 4  --indices_dir indices/glove_clean >> results/glove/bf3host_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 4  --indices_dir indices/glove_clean >> results/glove/bf3host_hnsw.txt  2>>&1 

    python3 index_searcher.py --only "flat"  --threads 2  --indices_dir indices/glove_clean >> results/glove/bf3host_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 2  --indices_dir indices/glove_clean >> results/glove/bf3host_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 2  --indices_dir indices/glove_clean >> results/glove/bf3host_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 2  --indices_dir indices/glove_clean >> results/glove/bf3host_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 2  --indices_dir indices/glove_clean >> results/glove/bf3host_hnsw.txt  2>>&1 

    python3 index_searcher.py --only "flat"  --threads 1  --indices_dir indices/glove_clean >> results/glove/bf3host_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 1  --indices_dir indices/glove_clean >> results/glove/bf3host_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 1  --indices_dir indices/glove_clean >> results/glove/bf3host_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 1  --indices_dir indices/glove_clean >> results/glove/bf3host_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 1  --indices_dir indices/glove_clean >> results/glove/bf3host_hnsw.txt  2>>&1

    #### Fasttext
    python3 index_searcher.py --only "flat"  --threads 16 --indices_dir indices/fasttext_final >> results/fasttext/bf3host_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 16 --indices_dir indices/fasttext_final >> results/fasttext/bf3host_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 16 --indices_dir indices/fasttext_final >> results/fasttext/bf3host_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 16 --indices_dir indices/fasttext_final >> results/fasttext/bf3host_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 16 --indices_dir indices/fasttext_final >> results/fasttext/bf3host_hnsw.txt  2>>&1

    python3 index_searcher.py --only "flat"  --threads 8  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 8  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 8  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 8  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 8  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_hnsw.txt  2>>&1
    
    python3 index_searcher.py --only "flat"  --threads 4  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 4  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 4  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 4  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 4  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_hnsw.txt  2>>&1
    
    python3 index_searcher.py --only "flat"  --threads 2  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 2  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 2  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 2  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 2  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_hnsw.txt  2>>&1
    
    python3 index_searcher.py --only "flat"  --threads 1  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_flat.txt  2>>&1
    python3 index_searcher.py --only "lsh"   --threads 1  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_lsh.txt   2>>&1
    python3 index_searcher.py --only "pq"    --threads 1  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_pq.txt    2>>&1
    python3 index_searcher.py --only "ivfpq" --threads 1  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_ivfpq.txt 2>>&1
    python3 index_searcher.py --only "hnsw"  --threads 1  --indices_dir indices/fasttext_final >> results/fasttext/bf3host_hnsw.txt  2>>&1

fi