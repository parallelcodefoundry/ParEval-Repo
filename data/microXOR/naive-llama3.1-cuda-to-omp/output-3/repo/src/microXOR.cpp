#include <omp.h>
#include "microXOR.hpp"

__attribute__((noinline)) void cellsXOR(int* input, int* output, size_t N) {
  #pragma omp parallel for collapse(2)
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      if (i > 0 && input[(i-1)*N + j] == 1) count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) count++;
      if (j > 0 && input[i*N + (j-1)] == 1) count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) count++;
      output[i*N + j] = (count == 1) ? 1 : 0;
    }
  }
}

int main(int argc, char **argv) {
  // ...
  
  int *d_input, *d_output;
  cudaMalloc(&d_input, N * N * sizeof(int));
  cudaMalloc(&d_output, N * N * sizeof(int));

  cudaMemcpy(d_input, input, N * N * sizeof(int), cudaMemcpyHostToDevice);

  #pragma omp offload target(device) \
    in (input[N*N], d_input[N*N]) \
    out (output[N*N], d_output[N*N])
  {
    cellsXOR<<<numBlocks, threadsPerBlock>>>(d_input, d_output, N);
  }

  cudaMemcpy(output, d_output, N * N * sizeof(int), cudaMemcpyDeviceToHost);

  // ...
}