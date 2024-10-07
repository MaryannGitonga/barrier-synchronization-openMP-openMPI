#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "gtmp.h"

int main(int argc, char** argv)
{
  int num_threads,num_iter=10;
  double start_time = 0, end_time = 0, time_diff = 0; // !added
  int pub = 0, thread_num = -1; // !added

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
#pragma omp parallel shared(num_threads) firstprivate(thread_num) // !added
  {
    thread_num = omp_get_thread_num(); // !added
    int i;
    for(i = 0; i < num_iter; i++){
      #pragma omp critical // !added
      {
        pub += thread_num;
      }

      // printf("thread %d: before barrier value of pub = %d\n", thread_num, pub); // !added
      gtmp_barrier();
      printf("thread %d: final value of pub = %d\n", thread_num, pub); // !added
    }

  }

  gtmp_finalize();

  // calculate time diff
  end_time = omp_get_wtime();
  time_diff = end_time - start_time;

  printf("Time taken: %2f seconds\n", time_diff);
  return 0;
}