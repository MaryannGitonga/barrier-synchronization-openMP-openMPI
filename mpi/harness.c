#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <mpi.h>
#include <time.h>
#include "gtmpi.h"

int main(int argc, char** argv)
{
  double time_diff_sum;
  struct timespec tstart, tend;
  double dt;

  int num_processes, my_id;
  int num_iter = 100;
  int exp_iter = 1e4;
  long total_time = 0;
  int pub = 0;

  MPI_Init(&argc, &argv);
  
  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_PROCS]\n");
    exit(EXIT_FAILURE);
  }

  if(argc > 2){ // to change number of iteration
    exp_iter = strtol(argv[2], NULL, 10);
  }

  num_processes = strtol(argv[1], NULL, 10);

  MPI_Comm_size(MPI_COMM_WORLD, &num_processes); // just in case execution differs with argc
  MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

  for(int j=0; j< exp_iter; j++){
    gtmpi_init(num_processes);
    clock_gettime(CLOCK_REALTIME, &tstart);

    /* ==============================================
    Parellel part
    ==============================================*/
    int i = 0;
    for(i=0; i < num_iter; i++){
      pub += my_id;  

      gtmpi_barrier();
    }

    /* ==============================================
    Timing check & clean up
    ==============================================*/
    clock_gettime(CLOCK_REALTIME, &tend);
    dt = (tend.tv_sec*1e6+tend.tv_nsec/1e3)-(tstart.tv_sec*1e6+tstart.tv_nsec/1e3);

    MPI_Reduce(&dt, &time_diff_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if(my_id == 0){
      total_time += time_diff_sum;
    }
    gtmpi_finalize();  
  }

  if(my_id == 0){
    // fprintf(stdout, "Total time taken for %d -> clock_gettime: %f μs\n", exp_iter, total_time/num_processes);
    fprintf(stdout, "Average time taken for %d experiments: %ld μs\n", exp_iter, (total_time/num_processes)/exp_iter);
    fprintf(stderr, "%d, %ld\n", num_processes, (total_time/num_processes)/exp_iter);
  }

  MPI_Finalize();
  return 0;
}
