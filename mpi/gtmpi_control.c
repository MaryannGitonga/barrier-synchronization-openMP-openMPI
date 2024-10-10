#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "gtmpi.h"

/*
    Control barrier (OpenMPI)
*/

int world_size;
void gtmpi_init(int num_processes){
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
}

void gtmpi_barrier(){
    MPI_Barrier(MPI_COMM_WORLD);
}

void gtmpi_finalize(){
}