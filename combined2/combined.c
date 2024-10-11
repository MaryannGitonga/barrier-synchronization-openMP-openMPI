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
Sense-reversing barrier
=============================================================*/
int s_P;
int count;
bool s_sense;
static bool local_sense = true;
#pragma omp threadprivate(local_sense)

void gtmpi_barrier();
void combined_init(int num_processes, int num_threads){
    /*=============================================================
    Sense-reversing barrier
    =============================================================*/
    s_P = num_threads;
    count = s_P;
    s_sense = true;

    /*=============================================================
    Tournament barrier
    =============================================================*/
    P = num_processes;
    num_tournament_rounds = (int)log2(P); 
    MPI_Comm_rank(MPI_COMM_WORLD, &vpid);    
    sense = true;

    tournament_rounds = (tournament_round_t **)calloc(P, sizeof(tournament_round_t *));
    for(int i=0; i< P; i++){
        tournament_rounds[i] = (tournament_round_t *)calloc(num_tournament_rounds+1, sizeof(tournament_round_t));
    }

    for(int i=0; i<P; i++){
        for(int k=0; k<num_tournament_rounds+1; k++){
            // init flag;
            tournament_rounds[i][k].flag = false;
            // init role;
            if(k > 0){ 
                if(i % (int)pow(2,k) == 0){ // bye or winner
                    if( (i + (int)pow(2,k-1) < P) && ((int)pow(2,k) < P)){ //  && not last round
                        tournament_rounds[i][k].role = winner;
                    } else if(i + (int)pow(2,k-1) >= P){ // not available in this round // !review again
                        tournament_rounds[i][k].role = bye;
                    }
                } else if(i % (int)pow(2,k) == (int)pow(2,k-1)){ // loser
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
    Sense-reversing barrier
    =============================================================*/
    local_sense = !local_sense; // each processor toggles its own sense
    #pragma omp critical // if fetch_and_decrement ($count) = 1
        {
            count--;
            if(count == 0){
                count = s_P;
                s_sense = local_sense; // last processor toggles global sense
            }
        }

    while(s_sense != local_sense);

    #pragma omp master
    {
        // printf("process%d:thread%d | call gtmpi_barrier() \n", node_id, thread_id);
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
}

void gtmpi_barrier(){ 
    int tournament_round = 1; // first tournament_round
    int exit_arrival = 1;

    // arrival loop
    while(exit_arrival){
        switch(tournament_rounds[vpid][tournament_round].role)
        {
            case loser:
                // printf("rounds[%d][%d].role = loser\n", vpid, tournament_round);
                MPI_Send(                
                    &sense, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD);
                // printf("rounds[%d][%d].role = loser | done, waiting handshake\n", vpid, tournament_round);
                MPI_Recv( 
                    &tournament_rounds[vpid][tournament_round].flag, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // printf("rounds[%d][%d].role = loser | handshaked, now leaving arrival\n", vpid, tournament_round);
                exit_arrival = 0; //exit loop
                break;
            case winner:
                // printf("rounds[%d][%d].role = winner\n", vpid, tournament_round);                
                MPI_Recv( 
                    &tournament_rounds[vpid][tournament_round].flag, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // printf("rounds[%d][%d].role = winner | recved msg, break\n", vpid, tournament_round);                
                break; // no need exit_arrival because champion will do in the last tournament_round
            case champion:
                // printf("rounds[%d][%d].role = champion\n", vpid, tournament_round);                
                MPI_Recv( 
                    &tournament_rounds[vpid][tournament_round].flag, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                // printf("rounds[%d][%d].role = champion | recved msg\n", vpid, tournament_round);                
                
                MPI_Send(                
                    &sense, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD);
                // printf("rounds[%d][%d].role = champion | start handshake, leaving arrival\n", vpid, tournament_round);                
                exit_arrival = 0; // exit loop
                break;
            case bye: // do nothing
                printf("rounds[%d][%d].role = bye\n", vpid, tournament_round);
            case dropout: // impossible
                break;
        }
        if(exit_arrival)
            tournament_round += 1; // move to next tournament_round
    }

    int exit_wakeup = 1;
    while(exit_wakeup){
        tournament_round -= 1;
        switch(tournament_rounds[vpid][tournament_round].role)
        {
            case winner:
                // printf("rounds[%d][%d].role = winner | exit | before handshake\n", vpid, tournament_round);
                MPI_Send(               
                    &sense, 1, MPI_C_BOOL, tournament_rounds[vpid][tournament_round].opponent, 0, MPI_COMM_WORLD);
                // printf("rounds[%d][%d].role = winner | exit | handshaked, now leaving\n", vpid, tournament_round);
                break;
            case dropout:
                exit_wakeup = 0; // exit loop when all tournament_round is done
                break;
            case loser: // impossible
            case bye: // do nothing
            case champion: // impossible
                break;
        }
    }

    sense = !sense; // reverse barrier
    // int thread_id = omp_get_thread_num();
    // printf("node%d:thread%d | round is done!=============================\n", vpid, thread_id);
}