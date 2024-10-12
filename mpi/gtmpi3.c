#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "gtmpi.h"

/*
    Control barrier (OpenMPI)
*/

void gtmpi_init(int num_processes){
}

void gtmpi_barrier(){
    MPI_Barrier(MPI_COMM_WORLD);
}

void gtmpi_finalize(){
}