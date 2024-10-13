#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <mpi.h>
#include <omp.h>
#include <time.h>
#include <string.h>
#include "combined.h"

int main(int argc, char** argv)
{
  double time_diff_sum;
  struct timespec tstart, tend;
  double dt;

  int num_processes, my_id;
  int num_threads;
  int thread_num = -1;
  int num_iter = 100;
  int exp_iter = 1000;
  double total_time = 0;
  int pub = 0;
    
  // debugging purpose
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;

  MPI_Init(&argc, &argv);
  
  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_THREADS]\n");
    exit(EXIT_FAILURE);
  }

  if(argc > 2){ // to change number of iteration
    exp_iter = strtol(argv[2], NULL, 10);
  }

  num_threads = strtol(argv[1], NULL, 10);
  omp_set_dynamic(0);
  if (omp_get_dynamic())
    printf("Warning: dynamic adjustment of threads has been set\n");

  omp_set_num_threads(num_threads);
  MPI_Comm_size(MPI_COMM_WORLD, &num_processes);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
  MPI_Get_processor_name(processor_name, &name_len); // Get the name of the processor

  for(int j=0; j< exp_iter; j++){
    combined_init(num_processes, num_threads);
    clock_gettime(CLOCK_REALTIME, &tstart);


    /* ==============================================
    Parellel part
    ==============================================*/
    int i = 0;
    #pragma omp parallel shared(pub) firstprivate(thread_num, i)
    {
      thread_num = omp_get_thread_num(); 
      for(i=0; i < num_iter; i++){
        #pragma omp critical 
        {
          pub += thread_num;
        }  

        combined_barrier();
      }
    }

    /* ==============================================
    Timing check & clean up
    ==============================================*/
    clock_gettime(CLOCK_REALTIME, &tend);
    dt = (tend.tv_sec*1e6+tend.tv_nsec/1e3)-(tstart.tv_sec*1e6+tstart.tv_nsec/1e3);

    // Gather time calcaulation from all processes
    // Reference: https://rookiehpc.org/mpi/docs/mpi_sum/index.html
    
    MPI_Reduce(&dt, &time_diff_sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  
    if(my_id == 0){
      total_time += time_diff_sum;
    }
    combined_finalize();  
  }

  if(my_id == 0){
    printf("process:%d&thread:%d | Total time taken for %d: %f μs\n",num_processes, num_threads, exp_iter, total_time/num_processes);

    printf("Average time taken for %d : %f μs\n", exp_iter, total_time/num_processes/exp_iter);
    fprintf(stderr, "%d, %d, %f\n", num_processes, num_threads, total_time/num_processes/exp_iter);
  }

  MPI_Finalize();
  return 0;
}

