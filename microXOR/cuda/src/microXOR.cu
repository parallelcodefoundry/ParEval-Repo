// microXOR kernel

#include <cuda_runtime.h>
#include "microXOR.cuh"

/* Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
   Note that we only consider neighbors and not input_{i,j} when computing output_{i,j}.
   input and output are NxN grids of ints in row-major.
   Use CUDA to compute in parallel. The kernel is launched on an NxN grid of threads.
   Example:

   input: [[0, 1, 1, 0],
           [1, 0, 0, 0],
           [0, 0, 0, 0],
           [0, 1, 0, 0]
   output: [[0, 0, 1, 1],
            [1, 0, 0, 1],
            [0, 0, 1, 0],
            [1, 0, 1, 0]]
*/
__global__ void cellsXOR(const int *input, int *output, size_t N) {
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
