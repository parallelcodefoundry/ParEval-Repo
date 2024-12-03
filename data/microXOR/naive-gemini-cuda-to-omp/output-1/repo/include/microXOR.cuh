// microXOR header file

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <omp.h>

#pragma omp declare target
void cellsXOR(const int *input, int *output, size_t N);
#pragma omp end declare target

#endif