#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gtmp.h"

/*
Dissemination Barrier

type flags = record
    myflags : array [0..1] of array [0..LogP-1] of Boolean
    partnerflags : array [0..1] of array [0..LogP-1] of ^Boolean

processor private parity : integer := 0
processor private sense : Boolean := true
processor private localflags : ^flags
shared allnodes : array [0..P-1] of flags
    // allnodes [1] is allocated in shared memory
    // locally accessible to processor i

// on processor i, localflags points to allnodes [i]
// initially allnodes [i].myflags[r][k] is false for all i, r, k
// if j = (i+2^k) mod P, then for r = O, 1:
    // allnodes[i].partnerflags[r][k] points to allnodes[j].myflags[r][k]


procedure dissemination_barrier
    for instance : integer := O to LogP-1
        localflags^.partnerflags[parity][instance]^ := sense
        repeat until localflags^.myflags[parity][instance] = sense

    if parity = 1
        sense := not sense
    parity := 1 - parity
*/

int **flags;
int n_threads;
int rounds;
static int parity = 0;
static int local_sense = 1;

#pragma omp threadprivate(parity, local_sense)

void gtmp_init(int num_threads){
    printf("....Dissemination barrier starting....\n");
    n_threads = num_threads;
    rounds = (int)ceil(log2(num_threads)); // calculate rounds

    // allocate memory for flags: flags[thread_id][parity][round]
    flags = (int**)malloc(num_threads * sizeof(int*));
    for (int i = 0; i < num_threads; i++)
    {
        flags[i] = (int*)malloc(2 * rounds * sizeof(int));
        for (int j = 0; j < 2 * rounds; j++)
        {
            flags[i][j] = 0; // init all flags to 0
        }
    }
}

void gtmp_barrier(){
    int thread_id = omp_get_thread_num();

    for (int round = 0; round < rounds; round++)
    {
        int peer = (thread_id + (1 << round)) % n_threads;
        flags[peer][parity * rounds + round] = local_sense;

        // spin on local sense until peer sends wake up call
        while (flags[thread_id][parity * rounds + round] != local_sense);
    }

    if (parity == 1)
    {
        local_sense = !local_sense;
    }
    
    parity = 1 - parity; // alternate parity
}

void gtmp_finalize(){
    for (int i = 0; i < n_threads; i++)
    {
        free(flags[i]);
    }

    free(flags);
    
    printf("....Dissemination barrier is done....\n");
}

