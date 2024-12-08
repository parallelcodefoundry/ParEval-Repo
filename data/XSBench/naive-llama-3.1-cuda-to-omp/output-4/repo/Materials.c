#include "XSbench_shared_header.h"

__global__ void compute_xs_kernel(double *x, double *y, long n_gridpoints) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx < n_gridpoints) {
    y[idx] = x[idx] * 2.0;
  }
}

void materials(void* args) {
  // Get the input arguments from the user
  Inputs in;
  memcpy(&in, args, sizeof(in));

  // Create a device memory pointer for the output data
  double *y_device;
  cudaMalloc((void **)&y_device, in.n_gridpoints * sizeof(double));

  // Launch the kernel on the GPU
  compute_xs_kernel<<<(in.n_gridpoints + 1023) / 1024, 1024>>>(x_host, y_device, in.n_gridpoints);
  cudaDeviceSynchronize();

  // Copy the output data from the device to the host
  double *y_host = (double*)malloc(in.n_gridpoints * sizeof(double));
  cudaMemcpy(y_host, y_device, in.n_gridpoints * sizeof(double), cudaMemcpyDeviceToHost);

  // Print the results
  printf("Results:\n");
  for (int i = 0; i < in.n_gridpoints; i++) {
    printf("%f\n", y_host[i]);
  }

  free(y_host);
}

int main() {
  Inputs in;
  memcpy(&in, &inputs, sizeof(in));

  // Create a CUDA stream
  cudaStream_t stream;
  cudaStreamCreate(&stream);

  // Set up the kernel launch parameters
  dim3 gridDim((in.n_gridpoints + 1023) / 1024);
  dim3 blockDim(1024);

  // Launch the kernel on the GPU
  materials<<<gridDim, blockDim>>>(NULL); // Use NULL as the input argument

  // Synchronize with the host thread
  cudaDeviceSynchronize();

  // Wait for the kernel to finish executing
  cudaStreamWaitEvent(stream, cuda_event);

  return 0;
}