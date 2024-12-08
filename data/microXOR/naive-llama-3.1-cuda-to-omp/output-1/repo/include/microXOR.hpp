#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>

// Define a macro for the number of threads in each direction
#define NUM_THREADS_X 32
#define NUM_THREADS_Y 32

#pragma offload_attribute(push)
#pragma offload_attribute(target(mic))
extern "C" {
void cellsXOR(int *input, int *output, size_t N);
}

// Kernel function declaration (not necessary with OpenMP offload)
// __global__ void cellsXOR(const int *input, int *output, size_t N);

#endif  // MICROXOR_CUH