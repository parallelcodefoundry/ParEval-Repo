// microXOR header file

#ifndef MICROXOR_HPP
#define MICROXOR_HPP

#include <iostream>
#include <random>
#include <omp.h>

//Declare the kernel function.  No special syntax needed for offloading.
void cellsXOR(const int *input, int *output, size_t N);

#endif