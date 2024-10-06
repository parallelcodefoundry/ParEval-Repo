#ifndef MICROXOR_HPP
#define MICROXOR_HPP

#include <iostream>
#include <random>

#define _OPENMP offload_target(target:mic)

extern "C" void cellsXOR(const int *input, int *output, size_t N);

#endif