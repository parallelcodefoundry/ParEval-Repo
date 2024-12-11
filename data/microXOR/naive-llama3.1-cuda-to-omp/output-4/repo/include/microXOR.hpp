#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>

#pragma offload_attribute(push, target devices(opencl:gpu))
__global__ void cellsXOR(const int *input, int *output, size_t N);
#pragma offload_attribute(pop)

#endif  // MICROXOR_CUH