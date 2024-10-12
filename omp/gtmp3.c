#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gtmp.h"

/*
Built-in Barrier
*/

void gtmp_init(int num_threads){
}

void gtmp_barrier(){
#pragma omp barrier
}

void gtmp_finalize(){
}

