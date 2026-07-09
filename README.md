This repo contains several directories:

**vector_similarity_test:** Our benchmarking pipeline for comparing FAISS ANN algorithms on the BF3 DPU and its host.

**Data:** Directory in which to install raw data files, also including some preprocessing code. Actual data files are not included in the repo due to size.

**mpi:** This directory contains programs that performs geospatial operations using MPI library. The work done by the programs and the installation information are in the first lines of the code files.

**spatial_operator, gRPC server-client example, tutorial_grpc_test, derda_grpc:** These directories relate to attempted experiments using gRPC for DPU offloading of geospatial operations with various setups.

**Documents, dma:** You may find some useful tools, survey papers and previous work about Geospatial Operations and Data Processing Units here.
