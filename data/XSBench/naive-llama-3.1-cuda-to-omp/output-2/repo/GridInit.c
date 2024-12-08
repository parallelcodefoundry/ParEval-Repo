// Modified from GridInit.cu, OpenMP-offload version

#include "XSbench_shared_header.h"

__global__ void InitGlobalVariables(Inputs *inputs) {
  int tid = threadIdx.x + blockIdx.x * blockDim.x;
  if (tid < inputs->nthreads) {
    // Initialize global variables for each thread
    GlobalVars[threadIdx.x] = inputs->HM[tid];
  }
}

int main(int argc, char **argv) {
  Inputs inputs;
  Profile profile;

  // Parse command-line arguments
  parse_args(argc, argv, &inputs);

  // Set up device and grid properties
  int num_devices = get_num_devices();
  int num_blocks_per_device = inputs.nthreads / num_devices + 1;
  dim3 grid_dim(num_blocks_per_device, 1);
  dim3 block_dim(256, 1); // Assuming 256 threads per block

  // Allocate global memory for device
  cudaMalloc((void **)&GlobalVars, inputs.nthreads * sizeof(float));

  // Launch kernel to initialize global variables
  InitGlobalVariables<<<grid_dim, block_dim>>>(inputs);

  // Wait for kernel to finish and copy results back to host
  cudaDeviceSynchronize();
  cudaMemcpy(&profile.device_to_host_time, GlobalVars, sizeof(double), cudaMemcpyDeviceToHost);

  // Measure execution time on device
  clock_t start = clock();
  kernel <<<grid_dim, block_dim>>>(inputs);
  cudaDeviceSynchronize();
  clock_t end = clock();

  // Calculate and print performance metrics
  profile.kernel_time = (double)(end - start) / CLOCKS_PER_SEC;
  print_profile(profile, inputs);

  return 0;
}