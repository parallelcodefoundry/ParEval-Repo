// microXOR header file

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <omp.h>

//The __global__ keyword is removed since we are not using CUDA.
void cellsXOR(const int *input, int *output, size_t N);

#endif