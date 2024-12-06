// microXOR kernel (OpenMP-Offload)

#include <omp.h>
#include "microXOR.cuh"

#define N 1024

__host__ void cellsXOR(const int *input, int *output) {
  size_t i, j;
  for (i = 0; i < N; ++i) {
    for (j = 0; j < N; ++j) {
      int count = 0;
      if (i > 0 && input[(i-1)*N + j] == 1) count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) count++;
      if (j > 0 && input[i*N + (j-1)] == 1) count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) count++;
      output[i*N + j] = (count == 1) ? 1 : 0;
    }
  }

  // Offload to GPU
#ifdef __cpp_lib_offload
  #pragma offload target(gpu) no_data_copy
  #pragma GCC ivdep
  __device__ void cellsXOR(const int *input, int *output) {
    size_t i, j;
    for (i = 0; i < N; ++i) {
      for (j = 0; j < N; ++j) {
        int count = 0;
        if (i > 0 && input[(i-1)*N + j] == 1) count++;
        if (i < N-1 && input[(i+1)*N + j] == 1) count++;
        if (j > 0 && input[i*N + (j-1)] == 1) count++;
        if (j < N-1 && input[i*N + (j+1)] == 1) count++;
        output[i*N + j] = (count == 1) ? 1 : 0;
      }
    }
  }

  #pragma offload target(gpu)
  cellsXOR(input, output);
#else
  // Fallback for compilers without OpenMP-Offload support
  #pragma GCC ivdep
  __host__ void cellsXOR(const int *input, int *output) {
    size_t i, j;
    for (i = 0; i < N; ++i) {
      for (j = 0; j < N; ++j) {
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
}