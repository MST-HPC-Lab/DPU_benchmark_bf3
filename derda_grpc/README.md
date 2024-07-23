# Instructions

This project is designed to use an installed version of gRPC as a library, rather than by including its source code within this project.

For getting and testing a local install of gRPC such that this project can work:

https://grpc.io/docs/languages/cpp/quickstart/

For running this project, those same instructions can work. Only, rather than running their examples/cpp/helloworld within the gRPC install files, change into this project's directory and follow the same idea. Something like this:

``` bash
# Change into the cmake directory to keep build files contained tidily
cd DPU_benchmark_bf3/derda_grpc/cmake

# Generate the build files for your machine / configuration
cmake .
# Should only need run the first time,
#   unless you add or remove files and edit CMakeLists.txt,
#   since cmake produces the Makefile.
# If this isn't working, gRPC or dependencies might not be installed properly.
#   Otherwise, consider trying 'cmake -DCMAKE_PREFIX_PATH=$GRPC_INSTALL_DIR`

# Compile it. Results are found in the `build` directory.
make
# Or `make -j 4` to split into multiple build jobs
```
