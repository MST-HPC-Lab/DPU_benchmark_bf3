#!/bin/bash
#SBATCH --job-name=dpu_test    # Job name
#SBATCH --mail-type=END,FAIL          # Mail events (NONE, BEGIN, END, FAIL, ALL)
#SBATCH --mail-user=derda.kaymak@marquette.edu     # Where to send mail  
#SBATCH --ntasks-per-node=8                   # Run on a single CPU
#SBATCH --time=10:00:00               # Time limit hrs:min:sec
#SBATCH --output=o_arm_mn.log   # Standard output and error log
pwd; hostname;


date
echo "Running on DPU"

export LD_LIBRARY_PATH=/global/home/users/satishp/geos_arm/build/lib
export HPCX_ARM_HOME=~/mpi/armMPI/hpcx-arm/
module use $HPCX_ARM_HOME/modulefiles
module load hpcx

echo "---- ARM BUILD ----"
mpic++ geosbm_mpi.cpp -o geosbm_arm -L/global/home/users/satishp/geos_arm/build/lib -lgeos_c -I/global/home/users/satishp/geos/usr/local/include
echo "---- ARM ----"
mpirun -np 1 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/sports_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 1 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 1 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 1 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 1 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 1 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/lakes_data 128
echo "-------------------"
mpirun -np 2 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/sports_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 2 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 2 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 2 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 2 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 2 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/lakes_data 128
echo "-------------------"
mpirun -np 4 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/sports_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 4 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 4 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 4 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 4 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 4 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/lakes_data 128
echo "-------------------"
mpirun -np 8 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/sports_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 8 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 8 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 8 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 8 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 8 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/lakes_data 128
echo "-------------------"
mpirun -np 16 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/sports_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 16 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 16 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 16 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 16 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 16 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/lakes_data 128
echo "-------------------"
mpirun -np 32 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/sports_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 32 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 32 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/lakes_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 32 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/cemet_data 128
mpirun -np 32 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/sports_data 128
mpirun -np 32 -npernode 1 ./geosbm_arm /global/scratch/users/satishp/data/building_128 /global/scratch/users/satishp/data/lakes_data 128
echo "-------------------"
