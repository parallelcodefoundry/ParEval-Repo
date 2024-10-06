#ifndef MICROXOR_HPP
#define MICROXOR_HPP

#include <iostream>
#include <random>

#define _OPENMP 
#include <omp.h>

extern void cellsXOR(const int *input, int *output, size_t N);

#endif