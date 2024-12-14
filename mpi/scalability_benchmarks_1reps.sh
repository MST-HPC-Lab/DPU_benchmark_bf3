mpirun -np 16 ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 1 > benchmark16x64x1.txt
mpirun -np 8  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 1 > benchmark8x64x1.txt
mpirun -np 4  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 1 > benchmark4x64x1.txt
mpirun -np 2  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 1 > benchmark2x64x1.txt
mpirun -np 8  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 32 1 > benchmark8x32x1.txt
mpirun -np 4  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 16 1 > benchmark4x16x1.txt
mpirun -np 2  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 8  1 > benchmark2x8x1.txt
echo "COMPLETED 7 SCALABILITY RUNS, 1 REPS"