// microXOR header file (translated to OpenMP-offload)

#ifndef MICROXOR_HPP
#define MICROXOR_HPP

#include <iostream>
#include <random>
#include <omp.h>

void cellsXOR(const int *input, int *output, size_t N);

#endif  // MICROXOR_HPP