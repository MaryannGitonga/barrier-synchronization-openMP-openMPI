#!/bin/bash

#SBATCH -J cs6210-proj2-mp1
#SBATCH -N 1
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice

echo "Started on `/bin/hostname`"

cd ~/omp

module load gcc/12.3.0 mvapich2/2.3.7-1
gcc gtmp1.c harness.c -o mp1 -g -std=gnu99 -I. -Wall -fopenmp -lm

# Run experiment across 2 to 8 threads
for threads in {2..8}
do
    echo "Running mp1 barrier with $threads threads"
    srun --cpus-per-task=$threads -o mp1_result_${threads}.out ./mp1 $threads 1000
done
