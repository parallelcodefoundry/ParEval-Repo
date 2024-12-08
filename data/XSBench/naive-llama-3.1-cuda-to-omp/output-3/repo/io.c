// io.cu (translated to OpenMP offloading)
#include "XSbench_shared_header.h"

void print_profile(Profile profile, Inputs in) {
  // ... same implementation as before ...
}

int main(int argc, char **argv) {
  // Initialize variables and input parameters
  Inputs inputs;
  // ... initialize inputs ...

  // Set up OpenMP environment for offloading
  omp_set_num_threads(inputs.nthreads);
  #pragma omp parallel
  {
    // Copy input parameters to each thread's private space
    Profile profile = {0.0};
    Inputs in = inputs;

    // Perform profiling
    #pragma omp master
    {
      if (in.filename) {
        FILE* output = fopen(in.filename, "w");
        fprintf(output, "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
      }
    }

    #pragma omp single nowait
    {
      // Initialize host-side data structures
      // ...

      // Perform kernel execution using offloaded loop
      #pragma omp taskloop default(none) firstprivate(profile, in)
      for (int i = 0; i < in.num_iterations; ++i) {
        // ... perform offloading here ...
      }
    }

    #pragma omp single nowait
    {
      // Copy results from device to host
      // ...

      // Print profile information
      print_profile(profile, in);
    }
  }

  return 0;
}