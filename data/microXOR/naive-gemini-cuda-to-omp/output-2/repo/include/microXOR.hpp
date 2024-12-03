// microXOR header file

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <omp.h>

#ifdef __cplusplus
extern "C" {
#endif

void cellsXOR(const int *input, int *output, size_t N);

#ifdef __cplusplus
}
#endif

#endif