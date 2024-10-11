#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <mpi.h>
#include <omp.h>
#include "combined.h"

void split_domain(int domain_size, int my_id, int num_processes, int* subdomain_start, int* subdomain_size);
void write_to_file(int my_id, int thread_id, int local_sum);

int main(int argc, char** argv)
{
  double start_time = 0, end_time = 0, time_diff = 0; // calculate time the barrier takes
  double *time_diff_sum = NULL;

  int num_threads;
  int thread_num = -1;
  int num_processes, my_id;

  MPI_Init(&argc, &argv);
  
  if (argc < 2){
    fprintf(stderr, "Usage: ./harness [NUM_THREADS]\n");
    exit(EXIT_FAILURE);
  }

  num_threads = strtol(argv[1], NULL, 10);
    omp_set_dynamic(0);

  if (omp_get_dynamic())
    printf("Warning: dynamic adjustment of threads has been set\n");

  omp_set_num_threads(num_threads);
  MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

  combined_init(num_processes, num_threads);

  MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  // Print off a hello world message
  printf("Hello world from processor %s, rank %d out of %d processors\n",
          processor_name, my_id, num_processes);
  MPI_Barrier(MPI_COMM_WORLD);

  if(my_id == 0){ 
    time_diff_sum = (double *)malloc(sizeof(double) * num_processes);
  }

  /* ==============================================
  Parellel part
  ==============================================*/
  int pub = 0;
  int i = 0;
  #pragma omp parallel shared(pub) firstprivate(thread_num, i)
  {
    for(i=0; i<3; i++){
      thread_num = omp_get_thread_num(); 
      #pragma omp critical 
      {
        pub += thread_num;
      }  

    combined_barrier(i); // passing i for debugging purpose
    printf("round%d:process%d:thread%d | pub = %d\n", i, my_id, thread_num, pub);
    combined_barrier(i); 
    }
  }
  /* ==============================================
  Parellel part
  ==============================================*/

  // performance check
  end_time = MPI_Wtime();
  time_diff = end_time - start_time;
  // printf("Time taken: %2f seconds\n", time_diff);
  MPI_Gather(&time_diff, 1, MPI_DOUBLE, time_diff_sum, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  if(my_id == 0){
    double time_result_sum = 0;
    for(int i=0; i<num_processes;i++){
      time_result_sum += time_diff_sum[i];
    }
    printf("Average Time taken: %2f seconds\n", time_result_sum/num_processes);
  }


  combined_finalize();  
  MPI_Finalize();

  free(time_diff_sum);
  return 0;
}

