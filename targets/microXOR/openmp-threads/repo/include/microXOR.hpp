// microXOR header file

#ifndef MICROXOR_HPP
#define MICROXOR_HPP

#include <iostream>
#include <random>
#include <vector>
#include <omp.h>

void cellsXOR(int* input, int* output, size_t N, size_t numThreads);

#endif
