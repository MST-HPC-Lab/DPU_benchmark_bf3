#!/bin/bash


if grep -q "BlueField" /sys/class/dmi/id/product_name; then # IS BLUEFIELD
    echo "DETECTED BLUEFIELD ENVIRONMENT"
else # NOT BLUEFIELD
    echo "DETECTED HOST ENVIRONMENT"

    if [ -d "indexes/glove" ]; then
        echo "INDEXES APPEAR TO ALREADY EXIST"
    else
        echo "BUILDING INDEXES"
        ./build_suite.sh
    fi

    echo "CHECKING FOR GROUND TRUTH PRESENCE"
    ./update_ground_truth.sh

    echo "COPYING INDEXES TO BLUEFIELD"
    # scp -r ./indexes/glove*/*.index ./indexes/glove*/*.json ./indexes/glove*/*.pkl ./indexes/glove*/x_query.npy ubuntu@192.168.100.2:~/DPU_benchmark_bf3/vector_similarity_test/
    # scp -r ./indexes/fasttext*/*.index ./indexes/fasttext*/*.json ./indexes/fasttext*/*.pkl ./indexes/fasttext*/x_query.npy ubuntu@192.168.100.2:~/DPU_benchmark_bf3/vector_similarity_test/
    # scp -r ./indexes/sift/*.index ./indexes/sift/*.json ./indexes/sift/*.pkl ./indexes/sift/x_query.npy ubuntu@192.168.100.2:~/DPU_benchmark_bf3/vector_similarity_test/
    # scp -r ./indexes/*/*.index ./indexes/*/*.json ./indexes/*/*.pkl ./indexes/*/x_query.npy ubuntu@192.168.100.2:~/DPU_benchmark_bf3/vector_similarity_test/
    scp -r ./indexes ubuntu@192.168.100.2:~/DPU_benchmark_bf3/vector_similarity_test/

    echo "STARTING SEARCHER ON BLUEFIELD"
    ssh ubuntu@192.168.100.2 "cd DPU_benchmark_bf3/vector_similarity_test; nohup ./combined_suite.sh > results/combined_suite_bf3.log 2>&1 &"
fi

echo "STARTING SEARCHER"
./search_suite.sh

echo "STARTING MULTIINDEXTEST"
./multitest_suite.sh

# if ! grep -q "BlueField" /sys/class/dmi/id/product_name; then # NOT BLUEFIELD
#     echo "COPYING RESULTS TO BF3"
#     scp results/results.json ubuntu@192.168.100.2:~/DPU_benchmark_bf3/vector_similarity_test/results/results.json
#     #
#     #
# fi

echo "ALL DONE!"