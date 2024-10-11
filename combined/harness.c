#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // added
#include <mpi.h>
#include <omp.h>
#include "combined.h"

void write_to_file(int my_id, int thread_id, int local_sum);

int main(int argc, char** argv)
{
  int num_threads;
  int thread_num = -1;
  int num_processes;
  int my_id;

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

// mpiexec.mpich -np 2 ./combined 4 => hang
// mpiexec.mpich -np 4 ./combined 4 => not hang
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

  // usleep(10);
  combined_barrier(i); // passing i for debugging purpose
  printf("round%d:process%d:thread%d | pub = %d\n", i, my_id, thread_num, pub);
  // usleep(10);
  combined_barrier(i); 
  }

}

  combined_finalize();  
  MPI_Finalize();

  return 0;
}

