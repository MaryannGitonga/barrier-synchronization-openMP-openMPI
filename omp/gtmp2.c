#include <omp.h>
#include "gtmp.h"
#include <stdio.h> 

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
int P;
int count;
bool sense;
static bool local_sense = true;
#pragma omp threadprivate(local_sense)

void gtmp_init(int num_threads){
    P = num_threads;
    count = P;
    sense = true;
}

void gtmp_barrier(){ 

    local_sense = !local_sense; // each processor toggles its own sense
    #pragma omp critical // if fetch_and_decrement ($count) = 1
        {
            count--;
            if(count == 0){
                count = P;
                sense = local_sense; // last processor toggles global sense
            }
        }

    while(sense != local_sense);
}

void gtmp_finalize(){
}