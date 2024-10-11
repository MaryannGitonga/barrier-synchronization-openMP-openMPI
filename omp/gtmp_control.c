#include <omp.h>
#include "gtmp.h"

/*
    Control barrier (OpenMP)
*/

void gtmp_init(int num_threads){
}

void gtmp_barrier(){
    #pragma omp barrier
}

void gtmp_finalize(){
}