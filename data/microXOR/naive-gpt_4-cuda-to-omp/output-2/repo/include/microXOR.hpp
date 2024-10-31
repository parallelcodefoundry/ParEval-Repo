// microXOR header file for OpenMP + GPU Offload

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>

// Declare the function prototype for the GPU offload version
#pragma omp declare target
void cellsXOR(const int *input, int *output, size_t N);
#pragma omp end declare target

#endif