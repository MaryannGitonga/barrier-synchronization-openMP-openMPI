#ifndef GTMP_H
#define GTMP_H

// !added
#define Boolean int
#define true 1
#define false 0

extern int P;
extern int count;
extern Boolean sense;
// !added

void gtmp_init(int num_threads);
void gtmp_barrier();				\
void gtmp_finalize();
#endif
