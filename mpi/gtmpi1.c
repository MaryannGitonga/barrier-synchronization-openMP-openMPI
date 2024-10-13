#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "gtmpi.h"

/*
    From the MCS Paper: A sense-reversing centralized barrier

    shared count : integer := P
    shared sense : Boolean := true
    processor private local_sense : Boolean := true

    procedure central_barrier
        local_sense := not local_sense // each processor toggles its own sense
    if fetch_and_decrement (&count) = 1
        count := P
        sense := local_sense // last processor toggles global sense
        else
            repeat until sense = local_sense
*/

static int counter;
static int shared_sense;
int world_size;

void gtmpi_init(int num_processes){
    counter = num_processes;
    shared_sense = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
}

void gtmpi_barrier(){
    int rank, local_sense;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    local_sense = !shared_sense;

    // Simulate fetch_and_increment with MPI_Allreduce to sum all the counters
    int total_count;
    MPI_Allreduce(&local_sense, &total_count, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    
    if (total_count == 1)
    {
        // if all processes have reached barrier, reset counter and flip sense flag
        counter = world_size;
        shared_sense = local_sense;
    } else {
        while (shared_sense != local_sense)
    }
}

void gtmpi_finalize(){
}