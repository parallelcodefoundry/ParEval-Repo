// microXOR header file

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <cuda_runtime.h>

__global__ void cellsXOR(const int *input, int *output, size_t N);

#endif
