#!/bin/bash
#SBATCH --job-name=dpu_test    # Job name
#SBATCH --mail-type=END,FAIL          # Mail events (NONE, BEGIN, END, FAIL, ALL)
#SBATCH --mail-user=derda.kaymak@marquette.edu     # Where to send mail  
#SBATCH --ntasks-per-node=8                   # Run on a single CPU
#SBATCH --time=10:00:00               # Time limit hrs:min:sec
#SBATCH --output=o_fp_x86.log   # Standard output and error log
pwd; hostname;


date
echo "Running on CPU"

export LD_LIBRARY_PATH=/global/home/users/satishp/geos/build/lib
module load gcc/8.3.1
module load hpcx/2.12.0

echo "---- ARM BUILD ----"
mpic++ bmfp.cpp -o bmfp_x86 -L/global/home/users/satishp/geos/build/lib -lgeos_c -I/global/home/users/satishp/geos/usr/local/include
echo "---- ARM ----"
# mpirun -np 1 -npernode 1 ./bmfp_x86 /global/scratch/users/satishp/data/sports_data /global/scratch/users/satishp/data/cemet_data 1
# mpirun -np 2 -npernode 2 ./bmfp_x86 /global/scratch/users/satishp/data/sports_data /global/scratch/users/satishp/data/cemet_data 1
# mpirun -np 4 -npernode 4 ./bmfp_x86 /global/scratch/users/satishp/data/sports_data /global/scratch/users/satishp/data/cemet_data 1
# mpirun -np 8 -npernode 8 ./bmfp_x86 /global/scratch/users/satishp/data/sports_data /global/scratch/users/satishp/data/cemet_data 1
# echo "-------------------"
# mpirun -np 1 -npernode 1 ./bmfp_x86 /global/scratch/users/satishp/data/lakes_data /global/scratch/users/satishp/data/cemet_data 1
# mpirun -np 2 -npernode 2 ./bmfp_x86 /global/scratch/users/satishp/data/lakes_data /global/scratch/users/satishp/data/cemet_data 1
# mpirun -np 4 -npernode 4 ./bmfp_x86 /global/scratch/users/satishp/data/lakes_data /global/scratch/users/satishp/data/cemet_data 1
# mpirun -np 8 -npernode 8 ./bmfp_x86 /global/scratch/users/satishp/data/lakes_data /global/scratch/users/satishp/data/cemet_data 1
# echo "-------------------"
# mpirun -np 1 -npernode 1 ./bmfp_x86 /global/scratch/users/satishp/data/lakes_data /global/scratch/users/satishp/data/sports_data 1
# mpirun -np 2 -npernode 2 ./bmfp_x86 /global/scratch/users/satishp/data/lakes_data /global/scratch/users/satishp/data/sports_data 1
# mpirun -np 4 -npernode 4 ./bmfp_x86 /global/scratch/users/satishp/data/lakes_data /global/scratch/users/satishp/data/sports_data 1
# mpirun -np 8 -npernode 8 ./bmfp_x86 /global/scratch/users/satishp/data/lakes_data /global/scratch/users/satishp/data/sports_data 1
echo "-------------------"
mpirun -np 1 -npernode 1 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/cemet_data 1
mpirun -np 2 -npernode 2 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/cemet_data 1
mpirun -np 4 -npernode 4 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/cemet_data 1
mpirun -np 8 -npernode 8 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/cemet_data 1
echo "-------------------"
mpirun -np 1 -npernode 1 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/sports_data 1
mpirun -np 2 -npernode 2 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/sports_data 1
mpirun -np 4 -npernode 4 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/sports_data 1
mpirun -np 8 -npernode 8 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/sports_data 1
echo "-------------------"
mpirun -np 1 -npernode 1 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/lakes_data 1
mpirun -np 2 -npernode 2 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/lakes_data 1
mpirun -np 4 -npernode 4 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/lakes_data 1
mpirun -np 8 -npernode 8 ./bmfp_x86 /global/scratch/users/satishp/data/building_data /global/scratch/users/satishp/data/lakes_data 1
echo "-------------------"
