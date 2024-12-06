#include <cuda_runtime.h>
#include "microXOR.cuh"

// Offload kernel function
void cellsXOR(const int *input, int *output, size_t N) {
  #pragma offload no_bounds
  __attribute__((target(gpu)))
  void launch(int i, int j) {
    if (i < N && j < N) {
      int count = 0;
      if (i > 0 && input[(i-1)*N + j] == 1) count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) count++;
      if (j > 0 && input[i*N + (j-1)] == 1) count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) count++;

      output[i*N + j] = (count == 1) ? 1 : 0;
    }
  }

  int numBlocksX = (N + get_num_threads_per_block().x - 1) / get_num_threads_per_block().x;
  int numBlocksY = (N + get_num_threads_per_block().y - 1) / get_num_threads_per_block().y;

  for(int i = 0; i < numBlocksX; ++i)
    for (int j = 0; j < numBlocksY; ++j) {
      launch(i * get_num_threads_per_block().x, j * get_num_threads_per_block().y);
    }
}

// Helper function to get number of threads per block
__device__ void set_num_threads_per_block(dim3 blockDim) {
  if (blockDim.x == blockDim.y) {
    int numThreadsPerBlock = 0;
    while ((1 << numThreadsPerBlock) <= blockDim.x) ++numThreadsPerBlock;
    get_num_threads_per_block() = dim3(numThreadsPerBlock, numThreadsPerBlock);
  }
}

// Helper function to get number of threads per block
__device__ void set_num_threads_per_block(dim3 blockDim, int numThreadsPerBlock) {
  if (blockDim.x == blockDim.y && numThreadsPerBlock <= blockDim.x) {
    get_num_threads_per_block() = dim3(numThreadsPerBlock, numThreadsPerBlock);
  }
}