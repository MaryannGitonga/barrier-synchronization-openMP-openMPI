#!/bin/bash

#SBATCH -J cs6210-proj2-mpi1
#SBATCH --ntasks-per-node=1
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice

echo "Started on `/bin/hostname`"

cd ~/mpi

module load gcc/12.3.0 mvapich2/2.3.7-1
mpicc gtmpi1.c harness.c -o mpi1 -g -Wall -std=gnu99 -I. -lm 

# Run experiment across 2 to 12 processes
for processes in {2..12}
do
    echo "Running sense-reversing barrier with $processes processes"
    srun -N $processes -o mpi1_np_${processes}.csv -e mpi1_np_${processes}.log ./mpi1 $processes 1000
done