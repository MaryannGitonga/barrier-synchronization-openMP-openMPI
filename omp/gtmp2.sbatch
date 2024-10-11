#!/bin/bash

#SBATCH -J cs6210-proj2-mp2
#SBATCH -N 1 --cpus-per-task=8
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice
#SBATCH -o sense_reversing_barrier_omp.out

echo "Started on `/bin/hostname`"


cd ~/omp

module load gcc/12.3.0 mvapich2/2.3.7-1
gcc gtmp2.c harness.c -o sense_reversing_barrier_omp -g -std=gnu99 -I. -Wall -fopenmp -lm

# Run experiment across 2 to 8 threads
for threads in {2..8}
do
    echo "Running sense-reversing barrier with $threads threads"
    srun sense_reversing_barrier_omp $threads
done
