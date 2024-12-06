#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <cuda_runtime.h>

void cellsXOR(const int *input, int *output, size_t N) {
  #pragma omp target teams distribute parallel for
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      int count = 0;
      if (i > 0 && input[(i-1)*N + j] == 1) count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) count++;
      if (j > 0 && input[i*N + (j-1)] == 1) count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) count++;
      output[i*N + j] = (count == 1) ? 1 : 0;
    }
  }
}

void offloadCellsXOR(const int *input, int *output, size_t N) {
  #pragma omp target teams distribute parallel for
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      int count = 0;
      if (i > 0 && input[(i-1)*N + j] == 1) count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) count++;
      if (j > 0 && input[i*N + (j-1)] == 1) count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) count++;
      output[i*N + j] = (count == 1) ? 1 : 0;
    }
  }
}

#endif