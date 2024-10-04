#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>

#if defined(__CUDA_ARCH__)
#include <cuda_runtime.h>
#endif

#if defined(__HIP_PLATFORM_HCC__)
#include <hip/hip_runtime.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void cellsXOR(const int *input, int *output, size_t N);

#ifdef __cplusplus
}
#endif

#endif