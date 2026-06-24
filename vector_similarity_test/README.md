## Instructions
1. Run index_builder.py first on host machine (once for each dataset, with separate output folder for each dataset or dataset version)
2. Copy indexes directories to each additional device
3. You may have to change the code to recognize your device instead of a BlueField-3, or to simply use hostname for naming the results json, so there's no merge conflict in the gathered data file.
4. Run multi_index_test.py on each device (Only supports GloVe). (Note that multi_index_test.py creates so many different versions of indexes that these are remade on the fly rather than stored, so resource-constrained devices may not be able to run it in full.)
5. Run search_suite.sh on each device
6. Run grapher files 
