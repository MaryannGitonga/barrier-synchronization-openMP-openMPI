#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>
#include "gtmpi.h"
/*
    From the MCS Paper: A scalable, distributed tournament barrier with only local spinning.
    
    type round_t = record
        role : (winner, loser, bye, champion, dropout)
        opponent : ^Boolean
        flag : Boolean
    shared rounds : array [0..P-1][0..LogP] of round_t
        // row vpid of rounds is allocated in shared memory
        // locally accessible to processor vpid
    processor private sense : Boolean := true
    processor private vpid : integer  // a unique virtual processor index
    
    // initially
    //    rounds[i][k].flag = false for all i,k
    // rounds[i][k].role =
    //    winner if k > 0, i mod (int)pow(2,k) = 0, i + (int)pow(2,k-1) < P, and (int)pow(2,k) < P
    //    bye if k > 0, i mod (int)pow(2,k) = 0, and i + (int)pow(2,k-1) >= P
    //    loser if k > 0 and i mod (int)pow(2,k) = (int)pow(2,k-1)
    //    champion if k > 0, i = 0, and (int)pow(2,k) >= P
    //    dropout if k = 0
    //    unused otherwise; value immaterial
    // rounds[i][k].opponent points to
    //    rounds[i-(int)pow(2,k-1)][k].flag if rounds[i][k].role = loser
    //    rounds[i+(int)pow(2,k-1)][k].flag if rounds[i][k].role = winner or champion
    //    unused otherwise; value immaterial
    
    procedure tournament_barrier
        round : integer := 1
        loop                // arrival
            case rounds[vpid][round].role of
                loser:
                    rounds[vpid][round].opponent^ := sense
                    repeat until rounds[vpid][round].flag = sense
                    exit loop
                winner:
                    repeat until rounds[vpid][round].flag = sense
                bye:        // do nothing
                champion:
                    repeat until rounds[vpid][round].flag = sense
                    rounds[vpid][round].opponent^ := sense
                    exit loop
                dropout:   // impossible
            round := round + 1
        loop                // wakeup
            round := round - 1
            case rounds[vpid][round].role of
                loser:      // impossible
                winner:
                    rounds[vpid][round].opponent^ := sense
                bye:        // do nothing
                champion:   // impossible
                dropout:
                    exit loop
        sense := not sense

*/
int P;
int num_rounds;

// shared among nodes
round_t **rounds;

// local to processor
int vpid;
bool sense;

void gtmpi_init(int num_processes){
    /*=============================================================
    init basic info
    =============================================================*/
    P = num_processes;
    num_rounds = get_num_rounds(P);
    MPI_Comm_rank(MPI_COMM_WORLD, &vpid);
    sense = true;

    /*=============================================================
    init rounds[vpid][round]
    =============================================================*/
    rounds = (round_t **)calloc(P, sizeof(round_t *));
    for(int i=0; i< P; i++){
        rounds[i] = (round_t *)calloc(num_rounds+1, sizeof(round_t));
    }

    /*=============================================================
    statically decide its role & its opponent
    =============================================================*/
    for(int i=0; i<P; i++){
        for(int k=0; k<num_rounds+1; k++){
            // init flag to false;
            rounds[i][k].flag = false;

            // init role;
            if(k > 0){ 
                if(i % (int)pow(2,k) == 0){ // bye or winner
                    if( (i + (int)pow(2,k-1) < P) && ((int)pow(2,k) < P)){ //  && not last round
                        rounds[i][k].role = winner;
                    } else if(i + (int)pow(2,k-1) >= P){ // not available in this round // !review again
                        rounds[i][k].role = bye;
                    }
                } else if(i % (int)pow(2,k) == (int)pow(2,k-1)){ // loser
                    rounds[i][k].role = loser;
                }
                if( (i == 0) && ((int)pow(2,k) >= P) ){ // champion when root proc && when last round
                    rounds[i][k].role = champion;
                }
            }else if(k==0){ // dropout
                rounds[i][k].role = dropout;
            }

            // init opponent;
            switch(rounds[i][k].role)
            {
                case loser: // loser point to its winner
                    rounds[i][k].opponent = i - (int)pow(2,k-1); // opponent to MPI_Send
                    break;
                case winner: // winner and champion point to its loser
                case champion: 
                    rounds[i][k].opponent = i + (int)pow(2,k-1);
                    break;        
                case dropout: // not needed
                    rounds[i][k].opponent = -1;
                    break;
                case bye: // not needed
                    rounds[i][k].opponent = -1;
                    break;
            }

        }
    }

}

void gtmpi_barrier(){ // MPI_Barrier(MPI_COMM_WORLD);
    int round = 1; // first round
    int exit_arrival = 1;

    // arrival loop
    while(exit_arrival){
        switch(rounds[vpid][round].role)
        {
            case loser:
                MPI_Send(                
                    &sense, 1, MPI_C_BOOL, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD);
                MPI_Recv( 
                    &rounds[vpid][round].flag, 1, MPI_C_BOOL, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                exit_arrival = 0; //exit loop
                break;
            case winner:
                MPI_Recv( 
                    &rounds[vpid][round].flag, 1, MPI_C_BOOL, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                break; // no need exit_arrival because champion will do in the last round
            case champion:
                MPI_Recv( 
                    &rounds[vpid][round].flag, 1, MPI_C_BOOL, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(                
                    &sense, 1, MPI_C_BOOL, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD);
                exit_arrival = 0; // exit loop
                break;
            case bye: // do nothing
            case dropout: // impossible
                break;
        }
        if(exit_arrival)
            round += 1; // move to next round
    }

    int exit_wakeup = 1;
    while(exit_wakeup){
        round -= 1;
        switch(rounds[vpid][round].role)
        {
            case winner:
                MPI_Send(               
                    &sense, 1, MPI_C_BOOL, rounds[vpid][round].opponent, 0, MPI_COMM_WORLD);
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

void gtmpi_finalize(){ 
    for(int i=0; i< P; i++)
        free(rounds[i]);
    free(rounds);
}

int get_num_rounds(int num_processes){
    return (int) (log((double) num_processes) / log(2.0));
}