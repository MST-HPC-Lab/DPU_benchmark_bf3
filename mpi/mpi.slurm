#!/bin/bash
#SBATCH --job-name=dpu_test    # Job name
#SBATCH --mail-type=END,FAIL          # Mail events (NONE, BEGIN, END, FAIL, ALL)
#SBATCH --mail-user=derda.kaymak@marquette.edu     # Where to send mail  
#SBATCH --ntasks-per-node=8                   # Run on a single CPU
#SBATCH --time=10:00:00               # Time limit hrs:min:sec
#SBATCH --output=out.log   # Standard output and error log
pwd; hostname;


date
echo "Running on CPIU + DPU"

export LD_LIBRARY_PATH=/global/home/users/satishp/geos_arm/build/lib

export HPCX_ARM_HOME=~/mpi/armMPI/hpcx-arm/

module use $HPCX_ARM_HOME/modulefiles
module load hpcx

echo "---- ARM BUILD ----"
mpic++ geosbm_mpi.cpp -o geosbm_arm -L/global/home/users/satishp/geos_arm/build/lib -lgeos_c -I/global/home/users/satishp/geos/usr/local/include
echo "---- ARM ----"
#mpirun -np 256 -npernode 1 ./geosbm_arm uniform-10k.wkt uniform-5.wkt
mpirun -np 8 -npernode 8 ./geosbm_arm /global/scratch/users/satishp/data_sp/lakes/2048Parts /global/scratch/users/satishp/data_sp/sports/2048Parts 2048
#mpirun -np 192 -npernode 8 ./geosbm_arm /global/scratch/users/satishp/data/lakes_8192 /global/scratch/users/satishp/data/cemet_8192 8192