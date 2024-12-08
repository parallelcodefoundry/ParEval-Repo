#include <omp.h>
#include "XSbench_shared_header.h"

__global__ void GridInitKernel(float *dev_XS, float *dev_grid, long ngridpoints, int grid_type) {
  int i = blockIdx.x * blockDim.x + threadIdx.x;
  if (i >= ngridpoints)
    return;

  switch(grid_type) {
    case UNIONIZEDGRID:
      dev_grid[i] = exp(dev_XS[i]);
      break;
    case NUCLEGRID:
      dev_grid[i] = exp(2*dev_XS[i]);
      break;
    default:
      printf("Invalid grid type\n");
      exit(1);
  }
}

void GridInit(float *X, float *grid, long ngridpoints, int grid_type) {
#pragma offload target(mic:0) \
  in(X:length=(ngridpoints), grid:length=ngridpoints) out(grid:length=ngridpoints) \
  map(to:X:length=(ngridpoints)) map(to:grid:length=ngridpoints)
  {
    int num_threads = 256;
    int num_blocks = (ngridpoints + num_threads - 1) / num_threads;

    GridInitKernel<<<num_blocks, num_threads>>>(X, grid, ngridpoints, grid_type);
#pragma offload target(mic:0) \
  in(grid:length=ngridpoints)
    {
      cudaDeviceSynchronize();
    }
  }
}