// microXOR header file

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <omp.h>

//This function declaration is modified to be compatible with OpenMP offloading
void cellsXOR(const int *input, int *output, size_t N);

#endif