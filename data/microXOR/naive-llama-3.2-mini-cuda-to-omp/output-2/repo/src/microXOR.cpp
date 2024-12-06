// microXOR kernel (offload)

#include <iostream>
#include <fstream>
#include <cuda_runtime.h>

#define N 1024 // adjust to your needs

void cellsXOR(const int *input, int *output, size_t N) {
  #pragma omp parallel for
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

void cellsXOROffload(const int *input, int *output, size_t N) {
  #pragma omp target offload
  __attribute__((accelerate)) {
    // Copy data from host to device (if needed)
    // cudaMemcpy(d_input, input, N * sizeof(int), cudaMemcpyHostToDevice);
    // cudaMemcpy(d_output, output, N * sizeof(int), cudaMemcpyDeviceToHost);

    cellsXOR(input, output, N);
  }
}