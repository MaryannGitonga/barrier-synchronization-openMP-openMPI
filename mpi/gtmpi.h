#include <stdbool.h>
#include <math.h>

#ifndef GTMPI_H
#define GTMPI_H

enum Role{winner=1, loser=2, bye=3, champion=4, dropout=5};

typedef struct{
    enum Role role;
    int opponent; // Boolean *opponent;
    bool flag;
} round_t;

extern round_t **rounds;
extern int vpid;
extern bool sense;
extern int P;
extern int num_rounds;


//! added
void gtmpi_init(int num_processes);
void gtmpi_barrier();
void gtmpi_finalize();

#endif