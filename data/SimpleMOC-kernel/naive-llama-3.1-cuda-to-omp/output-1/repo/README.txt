// OpenMP-Offload Execution Model: Overview
=====================================

This document provides an overview of how to use the OpenMP-offload execution model with our code.

### Introduction

The OpenMP-offload execution model allows you to execute OpenMP parallel regions offloaded to accelerators such as GPUs or Intel Xeon Phi processors. This model is particularly useful for large-scale computations that can benefit from the massively parallel architecture of these accelerators.

### Building and Running the Code

To build and run the code, follow these steps:

1. Install the OpenMP-offload toolkit by running `sudo apt-get install libopenmp-dev` (or equivalent command for your system).
2. Compile the code with the `-fopenmp` flag: `gcc -fopenmp -o myprogram myprogram.c`
3. Run the program using the `mpirun` command: `mpirun -np 4 ./myprogram`

### OpenMP-Offload Directives

To offload parallel regions to accelerators, use the following OpenMP-offload directives:

*   `#pragma omp target`: specifies the target accelerator (e.g., `omp_target(gpu)` or `omp_target(phi`)).
*   `#pragma omp teams`: creates a team of threads that will execute on the accelerator.
*   `#pragma omp task`: defines a task that can be executed by any thread in the team.

Example: