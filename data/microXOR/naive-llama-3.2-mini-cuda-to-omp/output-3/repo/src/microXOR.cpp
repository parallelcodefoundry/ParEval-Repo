// microXOR kernel (OpenMP offloading)

#include <omp.h>
#include "microXOR.cuh"

__device__ int cellsXOR(const int *input, int *output, size_t N) {
  int i = blockIdx.y * blockDim.y + threadIdx.y;
  int j = blockIdx.x * blockDim.x + threadIdx.x;
  if (i < N && j < N) {
    int count = 0;
    if (i > 0 && input[(i-1)*N + j] == 1) count++;
    if (i < N-1 && input[(i+1)*N + j] == 1) count++;
    if (j > 0 && input[i*N + (j-1)] == 1) count++;
    if (j < N-1 && input[i*N + (j+1)] == 1) count++;
    output[i*N + j] = (count == 1) ? 1 : 0;
  }
}

int main() {
  // Initialize data
  int N = 4; // grid size

  #pragma omp parallel for reduction(+:output[0]) num_threads(N*N)
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      __syncthreadid tid;
      __threadlocal int threadIdX = tid % N;
      __threadlocal int threadIdY = tid / N;

      if (threadIdX == 0 && threadIdY == 0) { // first element of the grid
        cellsXOR(&input[0], &output[0], N);
        break; // stop once one result is computed
      }
    }
  }

  return 0;
}