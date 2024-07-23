# Instructions

This project is designed to use an installed version of gRPC as a library, rather than by including its source code within this project.

For getting and testing a local install of gRPC such that this project can work:

https://grpc.io/docs/languages/cpp/quickstart/

For running this project, those same instructions can work. Only, rather than running their examples/cpp/helloworld within the gRPC install files, change into this project's directory and follow the same idea. Something like this:

``` bash
# Change into build directory (make it if it's missing) to keep build files contained tidily
cd DPU_benchmark_bf3/derda_grpc/build

# Generate the build files for your machine / configuration
cmake ../cmake
# Should only need run the first time,
#   unless you add or remove files and edit CMakeLists.txt,
#   since cmake produces the Makefile.
# If this isn't working, gRPC or dependencies might not be installed properly.
#   Otherwise, consider trying 'cmake -DCMAKE_PREFIX_PATH=$GRPC_INSTALL_DIR`
#   with whatever the gRPC install directory is (the BF3 and its host
#   currently have that variable set on login on the main user)

# Compile it. Results are found in the `bin` directory.
make
# Or `make -j 4` to split into multiple build jobs
```

If done this way, the resulting executables are placed in the `bin` folder, and interrim (throw-away / cached) build files in the `build` folder, with no clutter to your source or cmake directories.
