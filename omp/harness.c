#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "gtmp.h"

int main(int argc, char** argv)
{
  double start_time = 0, end_time = 0, time_diff = 0; 

  int num_threads;
  int thread_num = -1; 
  int num_iter=10;
  int pub = 0; 

  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_THREADS]\n");
    exit(EXIT_FAILURE);
  }
  num_threads = strtol(argv[1], NULL, 10);

  omp_set_dynamic(0);
  if (omp_get_dynamic())
    printf("Warning: dynamic adjustment of threads has been set\n");

  omp_set_num_threads(num_threads);

  gtmp_init(num_threads);
  start_time = omp_get_wtime();

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
      #pragma omp master
      {
        printf("round%d:thread%d | pub = %d\n", i, thread_num, pub);
      }
      gtmp_barrier();
    }
  }

  /* ==============================================
  Timing check & clean up
  ==============================================*/
  end_time = omp_get_wtime();
  time_diff = end_time - start_time;
  printf("Time taken: %2f seconds\n", time_diff);

  gtmp_finalize();

  return 0;
}