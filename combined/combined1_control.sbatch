#!/bin/bash

#SBATCH -J cs6210-proj2-combined1
#SBATCH --ntasks-per-node=1
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice
#SBATCH -o combined1_result.out

echo "Started on `/bin/hostname`"

cd ~/mpi

module load gcc/12.3.0 mvapich2/2.3.7-1
mpicc combined1.c harness.c -o combined1 -g -Wall -std=gnu99 -I. -lm 

# Run experiment across 2 to 12 processes
for processes in {2..12}; do
    for threads in {}; do
        echo "Running combined1 barrier with $processes processes $threads threads"
        srun -N=$processes --cpus-per-task=$threads ./combined1 $threads 1000 > combined1_result_${processes}_${threads}.out
    done
done