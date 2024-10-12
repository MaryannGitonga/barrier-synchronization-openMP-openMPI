#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <mpi.h>
#include <time.h>
#include "gtmpi.h"

int main(int argc, char** argv)
{
  // double start_time = 0, end_time = 0, time_diff = 0; // calculate time the barrier takes
  double *time_diff_sum = NULL;
  struct timespec tstart, tend;
  double dt;

  int num_processes, my_id;
  int num_iter = 100;
  int pub = 0;

  // debugging purpose
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;

  MPI_Init(&argc, &argv);
  
  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_PROCS]\n");
    exit(EXIT_FAILURE);
  }

  if(argc > 2){ // to change number of iteration
    num_iter = strtol(argv[2], NULL, 10);
  }

  num_processes = strtol(argv[1], NULL, 10);

  MPI_Comm_size(MPI_COMM_WORLD, &num_processes); // just in case execution differs with argc
  MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
  MPI_Get_processor_name(processor_name, &name_len); // Get the name of the processor

  // debugging purpose
  MPI_Barrier(MPI_COMM_WORLD);
  printf("Hello world from processor %s, rank %d out of %d processors\n",
          processor_name, my_id, num_processes);
  MPI_Barrier(MPI_COMM_WORLD);

  if(my_id == 0){ 
    time_diff_sum = (double *)malloc(sizeof(double) * num_processes);
  }

  gtmpi_init(num_processes);
  // start_time = MPI_Wtime();
  clock_gettime(CLOCK_REALTIME, &tstart);

  /* ==============================================
  Parellel part
  ==============================================*/
  int i = 0;
  for(i=0; i < num_iter; i++){
    pub += my_id;  

    gtmpi_barrier();
    printf("round%d:process%d | pub = %d\n", i, my_id, pub);
    gtmpi_barrier(); 
  }
  /* ==============================================
  Timing check & clean up
  ==============================================*/
  clock_gettime(CLOCK_REALTIME, &tend);
  dt = (tend.tv_sec*1e6+tend.tv_nsec/1e3)-(tstart.tv_sec*1e6+tstart.tv_nsec/1e3);

  // end_time = MPI_Wtime();
  // time_diff = end_time - start_time;
  // printf("Time taken: %2f seconds\n", dt);

  // Gather time calcaulation from all processes
  MPI_Gather(&dt, 1, MPI_DOUBLE, time_diff_sum, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  if(my_id == 0){
    double time_result_sum = 0;
    for(int i=0; i<num_processes;i++)
      time_result_sum += time_diff_sum[i];
    
    double average_time_diff = time_result_sum/num_processes;
    printf("Time taken: %.0f μs\n", average_time_diff);
  }

  gtmpi_finalize();  
  MPI_Finalize();
  free(time_diff_sum);
  return 0;
}
