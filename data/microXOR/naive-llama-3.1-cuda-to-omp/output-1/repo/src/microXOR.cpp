#include <omp.h>
#include "microXOR.cuh"

__attribute__((noinline)) int cellsXOR_offload(const int *input, int *output, size_t N) {
  #pragma omp offload target(nvptx64-nvidia-arch=sm_70) in(input[0:N*N]) out(output[0:N*N])
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