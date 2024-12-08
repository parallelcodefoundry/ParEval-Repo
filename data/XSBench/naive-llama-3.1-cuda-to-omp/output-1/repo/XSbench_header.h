// XSbench_shared_header.cpp
#ifndef XSBENCH_SHARED_HEADER_H
#define XSBENCH_SHARED_HEADER_H

// Header for shared utilities across XSBench versions

#include <omp.h>

typedef struct{
        int nthreads;
        long n_isotopes;
        long n_gridpoints;
        int lookups;
        char * HM;
        int grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
        int hash_bins;
        int particles;
        int simulation_method;
        int binary_mode;
        int kernel_id;
        int num_iterations;
        int num_warmups;
        char *filename;
} Inputs;

typedef struct{
  double device_to_host_time;
  double kernel_time;
  double host_to_device_time;
} Profile;

inline void print_profile(Profile profile, Inputs in) {
#pragma omp target teams distribute parallel for
  if (in.filename) {
    FILE* output = fopen(in.filename, "w");
    fprintf(output, "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
    fprintf(output, "%f,%f,%f,%d,%d\n",
            profile.host_to_device_time*1000,
            profile.kernel_time*1000,
            profile.device_to_host_time*1000,
            in.num_iterations,
            in.num_warmups);
    fclose(output);
  }
#pragma omp target teams distribute parallel for
  else {
    printf("host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
    printf("%f,%f,%f,%d,%d\n",
           profile.host_to_device_time*1000,
           profile.kernel_time*1000,
           profile.device_to_host_time*1000,
           in.num_iterations,
           in.num_warmups);
  }
}

#endif // XSBENCH_SHARED_HEADER_H


// XSbench_header.cpp
#ifndef XSBENCH_HEADER_H
#define XSBENCH_HEADER_H

#include "XSbench_shared_header.h"

__global__ void kernel1(Inputs in, Profile profile) {
    // Offload the kernel to a device (e.g., GPU)
#pragma offload target(device:cuda) in(in) out(profile)
  {
      // Kernel code here...
  }
}

__global__ void kernel2(Inputs in, Profile profile) {
    // Offload the kernel to a device (e.g., GPU)
#pragma offload target(device:cuda) in(in) out(profile)
  {
      // Kernel code here...
  }
}

// ...