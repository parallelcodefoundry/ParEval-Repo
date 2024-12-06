#ifndef XSBENCH_OFFLOAD_SHARED_HEADER_H
#define XSBENCH_OFFLOAD_SHARED_HEADER_H

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
    #pragma omp target map(in.num_iterations, in.num_warmups)
    {
        if (in.filename) {
            #pragma omp parallel for
            for (int i = 0; i < in.particles; ++i) {
                FILE* output = fopen(in.filename, "a");
                fprintf(output, "%f,%f,%f,%d,%d\n",
                        profile.host_to_device_time*1000,
                        profile.kernel_time*1000,
                        profile.device_to_host_time*1000,
                        in.num_iterations[i],
                        in.num_warmups[i]);
                fclose(output);
            }
        }
        else {
            #pragma omp parallel for
            for (int i = 0; i < in.particles; ++i) {
                printf("host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
                printf("%f,%f,%f,%d,%d\n",
                       profile.host_to_device_time*1000,
                       profile.kernel_time*1000,
                       profile.device_to_host_time*1000,
                       in.num_iterations[i],
                       in.num_warmups[i]);
            }
        }
    }
}

#endif // XSBENCH_OFFLOAD_SHARED_HEADER_H