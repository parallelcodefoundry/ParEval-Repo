#ifndef MICROXOR_HPP
#define MICROXOR_HPP

#include <iostream>
#include <random>

#define BLOCK_SIZE 32

void cellsXOR(const int *input, int *output, size_t N);

#endif