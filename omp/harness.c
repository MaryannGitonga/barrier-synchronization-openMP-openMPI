#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include "gtmp.h"

int main(int argc, char** argv)
{
  // double start_time = 0, end_time = 0, time_diff = 0; 
  struct timespec tstart, tend;
  double dt;

  int num_threads;
  int thread_num = -1; 
  int num_iter = 10;
  int exp_iter = 1e4;
  long total_time = 0;
  int pub = 0; 


  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_THREADS]\n");
    exit(EXIT_FAILURE);
  }
  num_threads = strtol(argv[1], NULL, 10);

  if(argc > 2){ // to change number of iteration
    num_iter = strtol(argv[2], NULL, 10);
  }

  omp_set_dynamic(0);
  if (omp_get_dynamic())
    printf("Warning: dynamic adjustment of threads has been set\n");

  omp_set_num_threads(num_threads);

  for (int j = 0; j < exp_iter; j++)
  {
    gtmp_init(num_threads);

    clock_gettime(CLOCK_REALTIME, &tstart);

    /* ==============================================
    Parellel part
    ==============================================*/  
    int i=0;
    #pragma omp parallel shared(pub) firstprivate(thread_num, i) 
    {
      thread_num = omp_get_thread_num();
      for(i=0; i < num_iter; i++){
        #pragma omp critical 
        {
          pub += thread_num;
        }

        gtmp_barrier();
        // #pragma omp master
        // {
        //   printf("round%d:thread%d | pub = %d\n", i, thread_num, pub);
        // }
        // gtmp_barrier();
      }
    }

    /* ==============================================
    Timing check & clean up
    ==============================================*/
    clock_gettime(CLOCK_REALTIME, &tend);
    
    // end_time = omp_get_wtime();
    dt = (tend.tv_sec*1e6+tend.tv_nsec/1e3)-(tstart.tv_sec*1e6+tstart.tv_nsec/1e3);
    // time_diff = (end_time - start_time) * 1e6;
    total_time += dt;

    gtmp_finalize();
  }

  printf("Average time taken for %d experiments: %ld μs\n", exp_iter, total_time/exp_iter);

  return 0;
}