// microXOR header file

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__HIP__) || defined(__NVCC__)
#  define __global__ __attribute__((device))
#else
#  define __global__
#endif

#if defined(__HIP__) || defined(__NVCC__)
#  define __host__ __attribute__((host))
#else
#  define __host__
#endif

__global__ void cellsXOR(const int *input, int *output, size_t N);

#ifdef __cplusplus
}
#endif

#endif