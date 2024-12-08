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