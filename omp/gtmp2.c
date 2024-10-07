#include <omp.h>
#include "gtmp.h"
#include <stdio.h> //!added

#define Boolean int
#define true 1
#define false 0

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
Boolean sense;
static Boolean local_sense = true;
#pragma omp threadprivate(local_sense)

void gtmp_init(int num_threads){
    // printf("[Serialized part] The number of threads is %d\n", num_threads);
    P = num_threads;
    count = P;
    sense = true;
}

void gtmp_barrier(){ // #pragma omp barrier
    // printf("[before]thread%d : local_sense = %d\n", omp_get_thread_num(), local_sense);

    local_sense = !local_sense; // each processor toggles its own sense
    #pragma omp critical // if fetch_and_decrement ($count) = 1
        {
            count--;
            if(count == 0){
                count = P;
                sense = local_sense; // last processor toggles global sense
            }
        }
    // printf("thread%d : local_sense = %d\n", omp_get_thread_num(), local_sense);

    while(sense != local_sense);
}

void gtmp_finalize(){
}