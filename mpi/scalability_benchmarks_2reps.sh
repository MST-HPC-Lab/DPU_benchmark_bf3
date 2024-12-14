mpirun -np 16 ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 2 > benchmark16x64x2.txt
mpirun -np 8  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 2 > benchmark8x64x2.txt
mpirun -np 4  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 2 > benchmark4x64x2.txt
mpirun -np 2  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 64 2 > benchmark2x64x2.txt
mpirun -np 8  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 32 2 > benchmark8x32x2.txt
mpirun -np 4  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 16 2 > benchmark4x16x2.txt
mpirun -np 2  ./geos_benchmark_mpi_spatial_partition ../Data/spatial_partition/sports/64_parts_sports ../Data/spatial_partition/cemetery/64_parts_cemetery 8  2 > benchmark2x8x2.txt
echo "COMPLETED 7 SCALABILITY RUNS, 2 REPS"