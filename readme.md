# Barrier Synchronization with OpenMP & OpenMPI

## Introduction
Barrier synchronization is essential in parallel computing to ensure threads or processes reach the same point in execution before continuing. This project investigates and evaluates various barrier synchronization algorithms in shared-memory (OpenMP), distributed-memory (MPI), and hybrid environments. The goal is to compare their scalability, efficiency, and performance under different workloads and configurations.

## Implemented Algorithms

### 1. Sense-Reversing Barrier
#### OpenMP:
- Threads have a private "sense" variable flipped after synchronization.
- A global "sense" variable is toggled when all threads reach the barrier.
- Threads proceed only when their private sense matches the global sense.

#### MPI:
- Processes spin on a global "sense" variable.
- Each process toggles its local sense and waits until the global sense matches.

This algorithm is simple but prone to contention on the shared/global sense variable, particularly in highly parallel systems.

### 2. Dissemination Barrier (OpenMP)
- Organizes threads into rounds of communication.
- In each round, a thread communicates with a specific partner.
- The number of rounds required is logarithmic to the number of threads (`ceil(log2P)`), where `P` is the thread count.

This barrier excels in scalability due to reduced synchronization overhead, as each thread interacts with only a subset of other threads in each round.

### 3. Tournament Barrier (MPI)
- Threads/processes are structured into a tournament-style hierarchy.
- Pairs of threads compete, and half advance to the next round.
- In the final round, a "champion" thread signals waiting threads in reverse order.

The tournament structure reduces direct contention on shared resources but introduces overhead in multi-round synchronization.

### 4. Combined Barrier (OpenMP + MPI)
- Combines OpenMP’s dissemination barrier and MPI’s tournament barrier.
- **Phase 1**: OpenMP threads synchronize using the dissemination barrier.
- **Phase 2**: The master thread uses the MPI tournament barrier to synchronize across nodes.

This approach enables synchronization in hybrid systems, where threads on multiple nodes coordinate their work.

## Experimental Setup

### Hardware
- **Cluster**: COC-ICE PACE high-performance cluster.
- **Nodes**: Each node has 12 cores, with an InfiniBand HDR100 connection for low-latency communication.
- **Configuration**:
  - OpenMP: Experiments ranged from 2 to 8 threads per node.
  - MPI: Experiments scaled from 2 to 12 nodes, with one process per node.
  - Combined: Ran on 2 to 8 nodes with 2 to 12 threads per process.

### Timing and Measurement
- **Tool**: `clock_gettime()` for high-precision timing.
- **Iterations**: Each barrier ran 10,000 iterations in a controlled test harness.
- **Metrics**: Average execution time per 10,000 iterations was logged for analysis.

## Results and Analysis

### OpenMP Barriers
1. **Dissemination Barrier**:
   - Best-performing barrier for OpenMP.
   - Execution time increased logarithmically as thread count grew, demonstrating excellent scalability.
   - Point-to-point communication minimizes contention, making it well-suited for multicore systems.

2. **Sense-Reversing Barrier**:
   - Significant performance degradation as thread count exceeded 6.
   - Relied on a shared global variable, causing contention and bottlenecks.

3. **Built-in OpenMP Barrier**:
   - Performance was between dissemination and sense-reversing barriers.
   - While more optimized than custom sense-reversing barriers, it did not match the specialized dissemination barrier.

### MPI Barriers
1. **Built-in MPI Barrier**:
   - Consistently the fastest across all process counts.
   - Benefited from MPI’s highly optimized internal synchronization mechanisms.

2. **Tournament Barrier**:
   - Outperformed the sense-reversing barrier by reducing contention through pairwise synchronization.
   - Still showed performance degradation at higher node counts due to increased rounds.

3. **Sense-Reversing Barrier**:
   - Poorest performance among MPI barriers.
   - Scaling was limited by contention on the global sense variable as node count increased.

### Combined Barriers (OpenMP + MPI)
1. **OpenMP Dissemination + MPI Tournament Barrier**:
   - Worst performance among combined barriers.
   - As node and thread counts increased, synchronization overhead and communication traffic compounded.

2. **OpenMP Sense-Reversing + MPI Tournament Barrier**:
   - Performed better than the dissemination + tournament combination in hybrid scenarios.
   - Flat execution time across thread counts but suffered in node scaling due to MPI tournament overhead.

3. **Built-in OpenMP + MPI Barrier**:
   - Best performance among combined barriers.
   - Demonstrated consistent scalability due to internal optimizations in the built-in synchronization mechanisms.

### Comparative Insights
- **OpenMP Dissemination Barrier**: Superior for shared-memory systems due to logarithmic communication.
- **Built-in MPI Barrier**: Most efficient for distributed-memory systems.
- **Hybrid Synchronization**: Introduced overhead that diminished performance compared to standalone barriers.
- **Global Variables**: Barriers relying on global synchronization (e.g., sense-reversing) were least scalable due to contention.

## Key Findings
1. Dissemination barriers outperform other OpenMP barriers in scalability for shared-memory systems.
2. MPI’s built-in barriers offer unmatched performance for distributed systems due to internal optimizations.
3. Combined barriers introduce synchronization overhead, making them less efficient than standalone barriers for large-scale systems.
4. Synchronization performance is influenced by architectural factors like memory contention, thread/process count, and interconnect bandwidth.

## References
1. [Mellor-Crummey, J. M., & Scott, M. L. (1991). *Algorithms for scalable synchronization on shared-memory multiprocessors*. ACM Transactions on Computer Systems.](https://doi.org/10.1145/103727.103729)
2. [ICE Cluster Resources Documentation, Georgia Institute of Technology.](https://gatech.service-now.com/home?id=kb_article_view&sysparm_article=KB0042095)
3. [gettimeofday(2) - Linux manual page.](https://man7.org/linux/man-pages/man2/gettimeofday.2.html)
4. [Krzyzanowski, P. (n.d.). Clock_gettime. pk.org.](https://people.cs.rutgers.edu/~pxk/416/notes/c-tutorials/gettime.html)
5. [A guide to using OpenMP and MPI on SeaWulf | Division of Information Technology.](https://it.stonybrook.edu/help/kb/a-guide-to-using-openmp-and-mpi-on-seawulf)