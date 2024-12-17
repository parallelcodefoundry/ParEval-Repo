// microXOR header file for OpenMP Offload

#ifndef MICROXOR_HPP
#define MICROXOR_HPP

#include <iostream>
#include <random>
#include <omp.h>

// Function prototype for the OpenMP offload version of cellsXOR
void cellsXOR(const int *input, int *output, size_t N);

#endif