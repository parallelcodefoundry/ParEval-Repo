// microXOR kernel for OpenMP Offload

#include <omp.h>
#include "microXOR.cuh"

/* Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
   Note that we only consider neighbors and not input_{i,j} when computing output_{i,j}.
   input and output are NxN grids of ints in row-major.
   Use OpenMP Offload to compute in parallel on the GPU.

   Parameters:
   input (const int *): pointer to the NxN grid of input values
   output (int *): pointer to the NxN grid of output values
   N (size_t): size of the grid

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
__attribute__((offload(threads_per_team:64))) __kernel void cellsXOR(const int *input, int *output, size_t N) {
  int i = get_global_id(1);
  int j = get_global_id(0);
  if (i < N && j < N) {
    int count = 0;
    if (i > 0 && input[(i-1)*N + j] == 1) count++;
    if (i < N-1 && input[(i+1)*N + j] == 1) count++;
    if (j > 0 && input[i*N + (j-1)] == 1) count++;
    if (j < N-1 && input[i*N + (j+1)] == 1) count++;
    output[i*N + j] = (count == 1) ? 1 : 0;
  }
}