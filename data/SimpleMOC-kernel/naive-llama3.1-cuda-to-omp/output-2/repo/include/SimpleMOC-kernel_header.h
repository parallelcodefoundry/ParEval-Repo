#ifndef __SIMPLE_MOC_KERNEL_HEADER__
#define __SIMPLE_MOC_KERNEL_HEADER__

#include <omp.h>

// Define the number of threads for OpenMP
#define NUM_THREADS 4

// Define a function to initialize device sources
void init_device_sources( Input I, Source_Arrays * SA_h, Source_Arrays * SA_d, Source * sources_h );

// Define a kernel function to perform computation on the device
__global__
void compute_kernel( Input I, Source_Arrays * SA_d ) {
    // Get the global thread ID
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    // Perform computation based on the input and source arrays
    if (tid < I.source_3D_regions) {
        // Access source array data using offloaded pointers
        float fine_source_val = SA_d->fine_source_arr[tid];
        float fine_flux_val = SA_d->fine_flux_arr[tid];
        float sigT_val = SA_d->sigT_arr[tid];

        // Perform computation using the accessed values
        // ...
    }
}

// Define a function to build an exponential table for linear interpolation
Table build_exponential_table( void );

#endif  // __SIMPLE_MOC_KERNEL_HEADER__