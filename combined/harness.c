#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // added
#include <mpi.h>
#include <omp.h>
#include "combined.h"

void split_domain(int domain_size, int my_id, int num_processes, int* subdomain_start, int* subdomain_size);
void write_to_file(int my_id, int thread_id, int local_sum);

int main(int argc, char** argv)
{
  int num_threads;
  int num_processes, num_rounds = 1;
  int my_id;
  double start_time = 0, end_time = 0, time_diff = 0; // calculate time the barrier takes
  double *time_diff_sum = NULL;

  int domain_size = 100;
  int subdomain_start = -1;
  int subdomain_size = -1;
  int subdomain_end = -1;
  int local_sum = 0; 
  int *total_sum = NULL; 

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
  start_time = MPI_Wtime();

  MPI_Comm_rank(MPI_COMM_WORLD, &my_id);

  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  // Print off a hello world message
  printf("Hello world from processor %s, rank %d out of %d processors\n",
          processor_name, my_id, num_processes);

  if(my_id == 0){ 
    time_diff_sum = (double *)malloc(sizeof(double) * num_processes);
    total_sum = (int *) malloc(sizeof(int) * num_processes);
  }

  int k;
  for(k = 0; k < num_rounds; k++){
    // split domain_size for each processes
    split_domain(domain_size, my_id, num_processes, &subdomain_start, &subdomain_size);
    subdomain_end = subdomain_start + subdomain_size -1;

    for(int i=num_processes-1; i>=0;i--){
      if(my_id == i){
  #pragma omp parallel for shared(local_sum) lastprivate(i)
        for(int i = subdomain_start; i <= subdomain_end; i++){
          usleep(rand() % 100);
          #pragma omp critical // !added
          {
            local_sum += i;
          }
        } // local summation
        combined_barrier(); // synchronize processes
      } // if my_id
    } // every processes run

    printf("process%d: local Summation = %d\n", my_id, local_sum);

    MPI_Gather(&local_sum, 1, MPI_INT, total_sum, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // performance check
    end_time = MPI_Wtime();
    time_diff = end_time - start_time;
    printf("Time taken: %2f seconds\n", time_diff);
    MPI_Gather(&time_diff, 1, MPI_DOUBLE, time_diff_sum, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if(my_id == 0){
      double time_result_sum = 0;
      int result_sum = 0;
      for(int i=0; i<num_processes;i++){
        result_sum += total_sum[i];
        time_result_sum += time_diff_sum[i];
      }
      printf("Total Summation = %d\n", result_sum);
      printf("Average Time taken: %2f seconds\n", time_result_sum/num_processes);
    }
  }

  combined_finalize();  
  MPI_Finalize();

  free(total_sum);
  free(time_diff_sum);

  return 0;
}

void write_to_file(int my_id, int thread_id, int local_sum){
  FILE *file;
  if((file = fopen("./summation_combined.txt", "a")) == NULL){
    perror("Couldn't open the file");
  }

  if(file != NULL){
    fprintf(file, "process%d:thread%d local_sum = %d\n", my_id, thread_id, local_sum);
    fclose(file);
  }

}
// Reference: https://mpitutorial.com/tutorials/point-to-point-communication-application-random-walk/
void split_domain(int domain_size, int my_id, int num_processes, int* subdomain_start, int* subdomain_size){
  if(num_processes > domain_size){ // if we can't share it
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  *subdomain_start = domain_size / num_processes * my_id + 1;
  *subdomain_size = domain_size / num_processes;
  if(my_id == num_processes -1){
    // Last process has all remainders
    *subdomain_size += domain_size % num_processes;
  }
}