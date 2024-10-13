#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include <stdio.h>
#include "combined.h"

/*=============================================================
Tournament barrier
=============================================================*/
int P; // num_processes
int vpid; // process id
int num_tournament_rounds;
tournament_round_t **tournament_rounds;
bool sense;


/*=============================================================
Dissemination barrier
=============================================================*/
static int rounds;
static int **flags;
static int n_threads;

void gtmpi_barrier();
void combined_init(int num_processes, int num_threads){
    /*=============================================================
    Dissemination barrier
    =============================================================*/
    n_threads = num_threads;
    rounds = (int)ceil(log2(num_threads)); // log2(P) rounds

    // allocate memory for flags: flags[thread_id][parity][round]
    flags = (int**)malloc(n_threads * sizeof(int*));
    for (int i = 0; i < n_threads; i++)
    {
        flags[i] = (int*)malloc(2 * rounds * sizeof(int));
        for (int j = 0; j < 2 * rounds; j++)
        {
            flags[i][j] = 0; // initialize all flags to 0
        }
    }

    /*=============================================================
    Tournament barrier
    =============================================================*/
    P = num_processes;
    num_tournament_rounds = ceil(log2(P));
    MPI_Comm_rank(MPI_COMM_WORLD, &vpid);
    sense = true;

    tournament_rounds = (tournament_round_t **)calloc(P, sizeof(tournament_round_t *));
    for(int i=0; i< P; i++){
        tournament_rounds[i] = (tournament_round_t *)calloc(num_tournament_rounds+1, sizeof(tournament_round_t));
    }

    for(int i=0; i<P; i++){
        for(int k=0; k<num_tournament_rounds+1; k++){
            // init flag to false;
            tournament_rounds[i][k].flag = false;

            // init role;
            if(k > 0){ 
                if(i % (int)pow(2,k) == 0){ // bye or winner
                    if( (i + (int)pow(2,k-1) < P) && ((int)pow(2,k) < P)){ //  && not last round
                        tournament_rounds[i][k].role = winner;
                    } else if(i + (int)pow(2,k-1) >= P){ // not available in this round // !review again
                        tournament_rounds[i][k].role = bye;
                    }
                } 

                if(i % (int)pow(2,k) == (int)pow(2,k-1)){ // loser
                    tournament_rounds[i][k].role = loser;
                }
                
                if( (i == 0) && ((int)pow(2,k) >= P) ){ // champion when root proc && when last round
                    tournament_rounds[i][k].role = champion;
                }
            }else if(k==0){ // dropout
                tournament_rounds[i][k].role = dropout;
            }

            // init opponent;
            switch(tournament_rounds[i][k].role)
            {
                case loser: // loser point to its winner
                    tournament_rounds[i][k].opponent = i - (int)pow(2,k-1); // opponent to MPI_Send
                    break;
                case winner: // winner and champion point to its loser
                case champion: 
                    tournament_rounds[i][k].opponent = i + (int)pow(2,k-1);
                    break;        
                case dropout: // not needed
                    tournament_rounds[i][k].opponent = -1;
                    break;
                case bye: // not needed
                    tournament_rounds[i][k].opponent = -1;
                    break;
            }
        }
    }
}

void combined_barrier(){
    /*=============================================================
    Dissemination barrier
    =============================================================*/
    int thread_id = omp_get_thread_num();
    int local_sense = 0;
    int parity = 0; // start with parity 0

    for (int round = 0; round < rounds; round++)
    {
        int partner = (thread_id + (1 << round)) % n_threads;

        // signal partner
        flags[partner][parity * rounds + round] = !local_sense;

        // spin on local sense until partner sends wake up call
        while (flags[thread_id][parity * rounds + round] == local_sense);
    }

    // flip local sense if parity is 1 after all rounds
    if (parity == 1)
    {
        local_sense = !local_sense;
    }
    parity = 1 - parity; // alternate parity

    #pragma omp master
    {
        gtmpi_barrier();
    }


}

void combined_finalize(){ 
    /*=============================================================
    Tournament barrier
    =============================================================*/
    for(int i=0; i< P; i++)
        free(tournament_rounds[i]);
    free(tournament_rounds);

    /*=============================================================
    Dissemination barrier
    =============================================================*/
    for (int i = 0; i < n_threads; i++)
    {
        free(flags[i]);
    }
    free(flags);
}

void gtmpi_barrier(){ // MPI_Barrier(MPI_COMM_WORLD);
    int tournament_round = 1; // first tournament_round
    int exit_arrival = 1;

    // arrival loop
    while(exit_arrival){
        switch(tournament_rounds[vpid][tournament_round].role)
        {
            case loser:
                MPI_Send(                
                    &sense, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD);
                MPI_Recv( 
                    &tournament_rounds[vpid][tournament_round].flag, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                exit_arrival = 0; //exit loop
                break;
            case winner:
                MPI_Recv( 
                    &tournament_rounds[vpid][tournament_round].flag, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                break; // no need exit_arrival because champion will do in the last round
            case champion:
                MPI_Recv( 
                    &tournament_rounds[vpid][tournament_round].flag, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(                
                    &sense, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD);
                exit_arrival = 0; // exit loop
                break;
            case bye: // do nothing
            case dropout: // impossible
                break;
        }
        if(exit_arrival)
            tournament_round += 1; // move to next round
    }

    int exit_wakeup = 1;
    while(exit_wakeup){
        tournament_round -= 1;
        switch(tournament_rounds[vpid][tournament_round].role)
        {
            case winner:
                MPI_Send(               
                    &sense, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD);
                break;
            case dropout:
                exit_wakeup = 0; // exit loop when all round is done
                break;
            case loser: // impossible
            case bye: // do nothing
            case champion: // impossible
                break;
        }
    }

    sense = !sense; // reverse barrier
}