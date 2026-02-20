#!/bin/bash
#SBATCH --job-name=dpu_test    # Job name
#SBATCH --mail-type=END,FAIL          # Mail events (NONE, BEGIN, END, FAIL, ALL)
#SBATCH --mail-user=derda.kaymak@marquette.edu     # Where to send mail  
#SBATCH --ntasks-per-node=8                   # Run on a single CPU
#SBATCH --time=10:00:00               # Time limit hrs:min:sec
#SBATCH --output=o_x86_sp.log   # Standard output and error log
pwd; hostname;


date
echo "Running on CPU"

export LD_LIBRARY_PATH=/global/home/users/satishp/geos/build/lib
module load gcc/8.3.1
module load hpcx/2.12.0

echo "---- X86 BUILD ----"
mpic++ geosbm_mpi.cpp -o geosbm_x86 -L/global/home/users/satishp/geos/build/lib -lgeos_c -I/global/home/users/satishp/geos/usr/local/include
echo "---- X86 ----"
mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/sports_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 2 -npernode 2 ./geosbm_x86 /global/scratch/users/satishp/data/sports_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 4 -npernode 4 ./geosbm_x86 /global/scratch/users/satishp/data/sports_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 8 -npernode 8 ./geosbm_x86 /global/scratch/users/satishp/data/sports_128 /global/scratch/users/satishp/data/cemet_data 128
echo "-------------------"
mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 2 -npernode 2 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 4 -npernode 4 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 8 -npernode 8 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/cemet_data 128
echo "-------------------"
mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 2 -npernode 2 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 4 -npernode 4 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 8 -npernode 8 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/sports_data 128
echo "-------------------"
mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 2 -npernode 2 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 4 -npernode 4 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 8 -npernode 8 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/cemet_data 128
echo "-------------------"
mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 2 -npernode 2 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 4 -npernode 4 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 8 -npernode 8 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/sports_data 128
echo "-------------------"
mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/lakes_data 128
mpirun -np 2 -npernode 2 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/lakes_data 128
mpirun -np 4 -npernode 4 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/lakes_data 128
mpirun -np 8 -npernode 8 ./geosbm_x86 /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/lakes_data 128
echo "-------------------"



# mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/sports_splitted /global/scratch/users/satishp/data/cemet_data 127
# mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/cemet_data 601
# mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/sports_data 601
# mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/cemet_data 8192
# mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/sports_data 8192
# mpirun -np 1 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/lakes_data 8192
# echo "-------------------"
# mpirun -np 2 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/sports_splitted /global/scratch/users/satishp/data/cemet_data 127
# mpirun -np 2 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/cemet_data 601
# mpirun -np 2 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/sports_data 601
# mpirun -np 2 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/cemet_data 8192
# mpirun -np 2 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/sports_data 8192
# mpirun -np 2 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/lakes_data 8192
# echo "-------------------"
# mpirun -np 4 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/sports_splitted /global/scratch/users/satishp/data/cemet_data 127
# mpirun -np 4 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/cemet_data 601
# mpirun -np 4 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/sports_data 601
# mpirun -np 4 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/cemet_data 8192
# mpirun -np 4 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/sports_data 8192
# mpirun -np 4 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/lakes_data 8192
# echo "-------------------"
# mpirun -np 8 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/sports_splitted /global/scratch/users/satishp/data/cemet_data 127
# mpirun -np 8 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/cemet_data 601
# mpirun -np 8 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/sports_data 601
# mpirun -np 8 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/cemet_data 8192
# mpirun -np 8 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/sports_data 8192
# mpirun -np 8 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/lakes_data 8192
# echo "-------------------"
# mpirun -np 16 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/sports_splitted /global/scratch/users/satishp/data/cemet_data 127
# mpirun -np 16 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/cemet_data 601
# mpirun -np 16 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/sports_data 601
# mpirun -np 16 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/cemet_data 8192
# mpirun -np 16 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/sports_data 8192
# mpirun -np 16 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/lakes_data 8192
# echo "-------------------"
# mpirun -np 32 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/sports_splitted /global/scratch/users/satishp/data/cemet_data 127
# mpirun -np 32 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/cemet_data 601
# mpirun -np 32 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/lakes_splitted /global/scratch/users/satishp/data/sports_data 601
# mpirun -np 32 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/cemet_data 8192
# mpirun -np 32 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/sports_data 8192
# mpirun -np 32 -npernode 1 ./geosbm_x86 /global/scratch/users/satishp/data/building_splitted /global/scratch/users/satishp/data/lakes_data 8192
# echo "-------------------"
