#!/bin/bash

#SBATCH -J cs6210-proj2-mpi2
#SBATCH --ntasks-per-node=1
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice
#SBATCH -o mpi2_result.out

echo "Started on `/bin/hostname`"

cd ~/mpi

module load gcc/12.3.0 mvapich2/2.3.7-1
mpicc gtmpi2.c harness.c -o mpi2 -g -Wall -std=gnu99 -I. -lm 

# Run experiment across 2 to 12 processes
for processes in {2..12}
do
    echo "Running tournament barrier with $processes processes"
    srun -N=$processes ./mpi2 $processes 1000 > mpi2_result_${processes}.out
done