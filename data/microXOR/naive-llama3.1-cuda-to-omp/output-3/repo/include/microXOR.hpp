// microXOR header file

#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>
#include <omp.h>

void cellsXOR(const int *input, int *output, size_t N) {
  #pragma omp declare target
  {
    #pragma omp parallel for
    for (size_t i = 0; i < N*N; i++) {
      int count = 0;
      if (i > 0 && input[(int)((i-1)/N)*N + (int)(i%N)] == 1) count++;
      if (i < N*N - 1 && input[(int)((i+1)/N)*N + (int)((i+1)%N)] == 1) count++;
      if (i > N && input[(int)((i-1)/N)*N + (int)((i-1)%N)] == 1) count++;
      if (i < N*N - N && input[(int)((i+N-1)/N)*N + (int)((i+N-1)%N)] == 1) count++;
      output[i] = (count == 1) ? 1 : 0;
    }
  }
}

#endif