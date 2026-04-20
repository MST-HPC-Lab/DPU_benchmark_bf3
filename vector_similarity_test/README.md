## Instructions
1. Run index_builder.py first on host machine (once for each dataset, with separate output folder for each dataset)
2. Copy indices directories to each additional device
3. Run multi_index_test.py on each device (TODO: Doesn't support other datasets yet), syncing between each so there's no merge conflict in the gathered data file (TODO: doesn't yet add to data file) (Note that multi_index_test.py creates so many different versions of indices that these are remade on the fly rather than stored, so resource-constrained devices may not be able to run it in full.)
4. Run search_suite.sh on each device, syncing between each so there's no merge conflict in the gathered data file
5. Run grapher files 