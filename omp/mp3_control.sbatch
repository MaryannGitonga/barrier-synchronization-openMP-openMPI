#!/bin/bash

#SBATCH -J cs6210-proj2-mp3
#SBATCH -N 1
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice
#SBATCH -o mp3_result.out

echo "Started on `/bin/hostname`"

cd ~/omp

module load gcc/12.3.0 mvapich2/2.3.7-1
gcc gtmp3.c harness.c -o mp3 -g -std=gnu99 -I. -Wall -fopenmp -lm

# Run experiment across 2 to 8 threads
for threads in {2..8}
do
    echo "Running mp3 barrier with $threads threads"
    srun --cpus-per-task=$threads ./mp3 $threads 1000 > mp3_result_${threads}.out
done
