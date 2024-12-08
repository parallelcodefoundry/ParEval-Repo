// microXOR kernel using OpenMP and offloading to GPU

#include <omp.h>
#include "microXOR.cuh"
#include <cuda_runtime.h>

#pragma omp declare target (gpu)

void cellsXOR(const int *input, int *output, size_t N) {
  #pragma omp parallel for
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      int count = 0;
      if (i > 0 && input[(i-1)*N + j] == 1) count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) count++;
      if (j > 0 && input[i*N + (j-1)] == 1) count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) count++;
      output[i*N + j] = (count == 1) ? 1 : 0;
    }
  }
}

// Launch the kernel on the GPU
void launch_kernel(const int *input, int *output, size_t N) {
  #pragma omp target map(to: input[0:N*N], output[0:N*N])
  {
    cellsXOR(input, output, N);
  }
}