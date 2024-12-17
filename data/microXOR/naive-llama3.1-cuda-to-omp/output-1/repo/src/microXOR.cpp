// microXOR kernel

#include <omp.h>
#include "microXOR.hpp"

#pragma offload_attribute(push)
#pragma offload_attribute(alloc, omp:parallel_for)
__attribute__((noinline))
void cellsXOR(int* input, int* output, size_t N) {
  #pragma offload target(mic)
  {
    int i = blockIdx.y * blockDim.y + threadIdx.y;
    int j = blockIdx.x * blockDim.x + threadIdx.x;

    if (i < N && j < N) {
      int count = 0;

      // Consider neighbors and not input_{i,j} when computing output_{i,j}.
      #pragma offload target(mic)
      {
        if (i > 0 && input[(i-1)*N + j] == 1) count++;
        if (i < N-1 && input[(i+1)*N + j] == 1) count++;
        if (j > 0 && input[i*N + (j-1)] == 1) count++;
        if (j < N-1 && input[i*N + (j+1)] == 1) count++;
      }

      output[i*N + j] = (count == 1) ? 1 : 0;
    }
  }
#pragma offload_attribute(pop)
}

// We now need to call the cellsXOR function from main using OpenMP parallel for directive.