#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <omp.h>

#pragma offload_attribute(push)
    #pragma offload_attribute(allocations)
        #include <cuda_runtime.h>
#pragma offload_attribute(pop)

__global__ void cellsXOR(const int *input, int *output, size_t N);

#endif