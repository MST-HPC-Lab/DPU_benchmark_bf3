#!/bin/bash
mpirun -np 16 ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 4 > benchmark16x64x4.txt
mpirun -np 8  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 4 > benchmark8x64x4.txt
mpirun -np 4  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 4 > benchmark4x64x4.txt
mpirun -np 2  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 4 > benchmark2x64x4.txt
mpirun -np 8  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 32 4 > benchmark8x32x4.txt
mpirun -np 4  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 16 4 > benchmark4x16x4.txt
mpirun -np 2  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 8  4 > benchmark2x8x4.txt
mpirun -np 16 ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_lakes    64 1 > benchmark16x64x1_lakes.txt
echo "COMPLETED 8 SCALABILITY RUNS, 4 REPS"