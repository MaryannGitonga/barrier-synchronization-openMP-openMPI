#!/bin/bash

#SBATCH -J cs6210-proj2-combined3
#SBATCH --ntasks-per-node=1
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice

echo "Started on `/bin/hostname`"

cd ~/combined

module load gcc/12.3.0 mvapich2/2.3.7-1
mpicc combined3.c harness.c -o combined3 -g -Wall -std=gnu99 -I. -lm 

# Run experiment across 2 to 12 processes
for processes in {2..8}; do
    for threads in {2..12}; do
        echo "Running combined3 barrier with $processes processes $threads threads"
        srun -N $processes --cpus-per-task=$threads -o combined3_result_${processes}_${threads}.out ./combined3 $threads 1000
    done
done