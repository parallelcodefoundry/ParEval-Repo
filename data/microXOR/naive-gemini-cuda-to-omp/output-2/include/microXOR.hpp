#ifndef MICROXOR_HPP
#define MICROXOR_HPP

#include <iostream>
#include <random>

#ifdef __cplusplus
extern "C" {
#endif

void cellsXOR(const int* input, int* output, size_t N);

#ifdef __cplusplus
}
#endif

#endif // MICROXOR_HPP