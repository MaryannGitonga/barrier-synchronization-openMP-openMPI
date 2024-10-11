#include <stdbool.h>

#ifndef GTMP_H
#define GTMP_H

extern int P;
extern int count;
extern bool sense;

void gtmp_init(int num_threads);
void gtmp_barrier();				
void gtmp_finalize();
#endif
