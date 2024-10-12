#!/bin/bash

#SBATCH -J cs6210-proj2-mpi3
#SBATCH --ntasks-per-node=1
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice

echo "Started on `/bin/hostname`"

cd ~/mpi

module load gcc/12.3.0 mvapich2/2.3.7-1
mpicc gtmpi3.c harness.c -o mpi3 -g -Wall -std=gnu99 -I. -lm 

# Run experiment across 2 to 12 processes
for processes in {2..12}
do
    echo "Running built-in barrier with $processes processes"
    srun -N $processes ./mpi3 $processes 1000 > mpi3_result_${processes}.out
done