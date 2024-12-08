// microXOR header file for OpenMP Offload

#ifndef MICROXOR_H
#define MICROXOR_H

#include <iostream>
#include <random>
#include <omp.h>

void cellsXOR(const int *input, int *output, size_t N);

#endif