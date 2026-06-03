if grep -q "BlueField" /sys/class/dmi/id/product_name; then
    echo "DETECTED BLUEFIELD; DISCONTINUING!"

else
    echo "BUILDING GLOVE INDEXES"
    python3 index_builder.py --file "glove.6B.200d.txt" --out_folder "glove"

    echo "BUILDING GLOVE EQUAL INDEXES"
    python3 index_builder.py --file "glove.6B.200d.txt" --out_folder "glove_128d_400k" --num_vecs 400000 --dim 128

    echo "BUILDING FASTTEXT INDEXES"
    python3 index_builder.py --file "fasttext_cc.en.300.vec" --out_folder "fasttext"

    echo "BUILDING FASTTEXT EQUAL INDEXES"
    python3 index_builder.py --file "fasttext_cc.en.300.vec" --out_folder "fasttext_128d_400k" --num_vecs 400000 --dim 128

    echo "BUILDING SIFT10M INDEXES"
    python3 index_builder.py --file "sift10m.txt" --out_folder "sift"

    echo "BUILDING SIFT10M EQUAL INDEXES"
    python3 index_builder.py --file "sift10m.txt" --out_folder "sift_128d_400k" --num_vecs 400000 --dim 128

    echo "BUILDING SIFT10M INCREASING INDEXES 100k"
    python3 index_builder.py --file "sift10m.txt" --out_folder "sift_100k" --num_vecs 100000
    echo "BUILDING SIFT10M INCREASING INDEXES 200k"
    python3 index_builder.py --file "sift10m.txt" --out_folder "sift_200k" --num_vecs 200000
    echo "BUILDING SIFT10M INCREASING INDEXES 500k"
    python3 index_builder.py --file "sift10m.txt" --out_folder "sift_500k" --num_vecs 500000
    echo "BUILDING SIFT10M INCREASING INDEXES 1m"
    python3 index_builder.py --file "sift10m.txt" --out_folder "sift_1m" --num_vecs 1000000
    echo "BUILDING SIFT10M INCREASING INDEXES 2m"
    python3 index_builder.py --file "sift10m.txt" --out_folder "sift_2m" --num_vecs 2000000
    echo "BUILDING SIFT10M INCREASING INDEXES 5m"
    python3 index_builder.py --file "sift10m.txt" --out_folder "sift_5m" --num_vecs 5000000
    echo "BUILDING SIFT10M INCREASING INDEXES 10m"
    python3 index_builder.py --file "sift10m.txt" --out_folder "sift_10m" --num_vecs 10000000

fi