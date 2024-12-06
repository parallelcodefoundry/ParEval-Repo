#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <cuda_runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

void cellsXOR(const int *input, int *output, size_t N, void (*kernel_func)(const int *, int *, size_t));

#ifdef __cplusplus
}
#endif

#endif // MICROXOR_CUH