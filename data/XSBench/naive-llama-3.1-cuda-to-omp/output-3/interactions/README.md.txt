Here is the translation of the provided README.md file into a C/C++ file that utilizes OpenMP and Offloading:
```c
#include "XSbench_shared_header.h"

// Function definitions for offloaded kernels
void kernel_offload_kernel_1(int *A, int N) {
  #pragma omp target teams distribute parallelize(N)
    for (int i = 0; i < N; i++) {
      A[i] *= 2;
    }
}

void kernel_offload_kernel_2(int *A, int N) {
  #pragma omp target teams distribute parallelize(N)
    for (int i = 0; i < N; i++) {
      A[i] += 1;
    }
}

// Function definitions for host-side operations
void init_particles(Inputs in) {
  // Initialize particle data on the host
  printf("Initializing %d particles...\n", in.particles);
}

void finalize_particles(Inputs in) {
  // Perform final operations on the host (e.g., output results)
  printf("Finalizing %d particles...\n", in.particles);
}

// Main function for offloading and executing kernels
int main(int argc, char **argv) {
  Inputs in;
  Profile profile;

  // Initialize inputs from command-line arguments or other sources
  in.filename = argv[1];

  init_particles(in);

  // Offload initialization to host-side
  #pragma omp target teams distribute parallelize(in.num_iterations)
    for (int i = 0; i < in.num_iterations; i++) {
      kernel_offload_kernel_1(in.HM, in.nthreads);
      kernel_offload_kernel_2(in.HM, in.nthreads);
    }

  // Wait for offloaded kernels to complete
  #pragma omp target wait

  finalize_particles(in);

  print_profile(profile, in);

  return 0;
}
```
This code defines three functions:

1. `kernel_offload_kernel_1` and `kernel_offload_kernel_2`: These are the two offloaded kernels that perform computations on the device.
2. `init_particles`: Initializes particle data on the host side.
3. `finalize_particles`: Performs final operations on the host side (e.g., output results).

The main function initializes inputs from command-line arguments or other sources, initializes particles on the host side using `init_particles`, offloads initialization to the device using OpenMP directives, waits for the offloaded kernels to complete using `omp_target_wait`, and finally prints the profile information.

Note that this code assumes you have a basic understanding of OpenMP and Offloading concepts. If you need further clarification or modifications, please let me know!