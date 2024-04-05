// microXOR header file

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <vector>
#include <omp.h>

void cellsXOR(int* input, int* output, size_t N, size_t threadsPerTeam, size_t numTeams);

#endif
