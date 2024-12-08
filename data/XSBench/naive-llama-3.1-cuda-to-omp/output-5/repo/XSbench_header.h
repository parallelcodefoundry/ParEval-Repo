#ifndef XS_BENCH_H
#define XS_BENCH_H

// Header for XSBench

#ifdef __CUDA_ARCH__
    #define CUDA_HOST_DEVICE
#endif

#if defined(ENABLE_PAPI) && defined(OPENMP_OFFLOAD)
    #pragma offload target(mic:offload)
    #include <omp.h>
#else
    #ifndef _OPENMP
        #error OpenMP not found, please install it
    #endif
#endif

// Define functions to be used by host and device code

__host__ __device__ double get_time();

inline void print_profile(Profile profile, Inputs in);

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

#endif // XS_BENCH_H