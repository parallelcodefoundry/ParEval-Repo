#include "XSbench_shared_header.h"

// Define a macro for offloading to a GPU device
#define OFFLOAD_TO_DEVICE(func) \
  __attribute__((offload(device, func)))

// Offload function to perform nuclide lookups on a GPU device
void OFFLOAD_TO_DEVICE(perform_nuclide_lookups)(double *E_in, double *X_in, double *Y_in, Inputs in) {
  #pragma offload target(mic:0)
  int i;
  for (i = 0; i < in.nthreads; ++i) {
    // Perform nuclide lookups on the GPU device
    perform_nuclide_lookups_kernel<<<1, 256>>>(E_in[i], X_in[i], Y_in[i]);
  }
}

// Offload function to initialize variables for a kernel launch
void OFFLOAD_TO_DEVICE(kernel_init_vars)(int i) {
  #pragma offload target(mic:0)
  // Initialize variables for the kernel launch on the GPU device
}

// Main function
void io(void *args) {
  Inputs in = *(Inputs*) args;
  
  // Perform offloaded work on a GPU device
  perform_nuclide_lookups(E_in, X_in, Y_in, in);
  
  // Copy data from the GPU device back to host memory
  cudaMemcpy(host_mem, gpu_mem, sizeof(double), cudaMemcpyDeviceToHost);
  
  // Call other offloaded functions as needed
}