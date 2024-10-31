// microXOR header file for OpenMP + GPU Offload

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>

// Define the cellsXOR function to be used with OpenMP target offload
void cellsXOR(const int *input, int *output, size_t N);

#endif