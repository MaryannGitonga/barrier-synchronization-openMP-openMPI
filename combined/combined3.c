#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include "combined.h"

void combined_init(int num_processes, int num_threads){
}

void combined_barrier(){
    #pragma omp barrier 
    #pragma omp master
    {
        MPI_Barrier(MPI_COMM_WORLD);
    }


}

void combined_finalize(){ 
}

