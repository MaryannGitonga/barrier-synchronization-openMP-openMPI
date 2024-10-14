#!/bin/bash

#SBATCH -J cs6210-proj2-mpi1
#SBATCH -N 12 --ntasks-per-node=1
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice
#SBATCH -o tournament_barrier_mpi.out

echo "Started on `/bin/hostname`"


cd ~/mpi

module load gcc/12.3.0 mvapich2/2.3.7-1
mpicc gtmpi2.c harness.c -o tournament_barrier_mpi -g -Wall -std=gnu99 -I. -lm 

for processes in {2..12}; do
    echo "Running tournament barrier with $processes processes"
    srun tournament_barrier_mpi $processes
done