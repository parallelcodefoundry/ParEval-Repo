#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>

#define restrict __restrict__

extern "C" void cellsXOR(const int *restrict input, int *restrict output, size_t N);

#endif