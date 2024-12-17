#include "microXOR.hpp"
#include <omp.h>
#include <iostream>

// Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
//
// Note that we only consider neighbors and not input_{i,j} when computing output_{i,j}.
// input and output are NxN grids of ints in row-major.
//
// Use OpenMP-offload to compute in parallel on the GPU. The kernel is launched on an NxN grid of threads.
//
// Example:

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

// This function launches the OpenMP-offload kernel
void launchCellsXOR(const int *input, int *output, size_t N) {
  // Define the number of threads per block and number of blocks
  const size_t blockDim = 16; // Replace with desired thread count
  const size_t gridSize = (N + blockDim - 1) / blockDim;

  // Offload to GPU using OpenMP-offload directive
#pragma offload target(mic:0) in(input[:N*N]) out(output[:N*N])
  cellsXOR<<<gridSize, blockDim>>>(input, output, N);
}

// Launch the kernel with input data and grid size
void microXOR(const int *input, int *output, size_t N) {
  launchCellsXOR(input, output, N);
}