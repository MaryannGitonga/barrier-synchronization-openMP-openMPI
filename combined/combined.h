#include <stdbool.h>
#include <math.h>

#ifndef COMBINED_H
#define COMBINED_H

enum Role{winner=1, loser=2, bye=3, champion=4, dropout=5};

typedef struct{
    enum Role role;
    int opponent; 
    bool flag;
} tournament_round_t;

extern tournament_round_t **tournament_rounds;
extern int vpid;
extern bool sense;
extern int P;
extern int num_tournament_rounds;

// OpenMP
extern int P;
extern int count;
extern bool sense;

void combined_init(int num_processes, int num_threads);
void combined_barrier(int round_cnt);
void gtmpi_barrier();
void combined_finalize();

#endif
