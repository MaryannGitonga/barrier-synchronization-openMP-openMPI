#!/bin/bash

#SBATCH -J cs6210-proj2-combined2
#SBATCH --ntasks-per-node=1
#SBATCH --mem-per-cpu=1G
#SBATCH -t 5
#SBATCH -q coc-ice
#SBATCH -o combined_barrier_sense_tourna.out

echo "Started on `/bin/hostname`"

cd ~/combined

module load gcc/12.3.0 mvapich2/2.3.7-1
mpicc combined2.c harness.c -o combined2 -g -Wall -std=gnu99 -I. -lm 

for processes in {2,4,8}; do
    for threads in {2..12}; do
        echo "Running combined2 barrier with $processes processes $threads threads"
        srun -N $processes ./combined2 $threads 1000
    done
done