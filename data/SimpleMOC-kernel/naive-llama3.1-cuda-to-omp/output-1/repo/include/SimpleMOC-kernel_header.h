#ifndef _SIMPLE_MOC_KERNEL_HEADER_H_
#define _SIMPLE_MOC_KERNEL_HEADER_H_

// CUDA-related headers
#include <cuda_runtime.h>
#include <cuda_fp.h>

// Offload-related headers (OpenMP)
#include <omp.h>

// Standard library headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define CUDA_ERROR_CHECK

// Define a structure to hold device arrays
typedef struct {
    float *fine_source_arr;
    float *fine_flux_arr;
    float *sigT_arr;
} Source_Arrays;

// Function declarations
Source* initialize_device_sources(Input I, Source_Arrays* SA_h, Source_Arrays* SA_d, Source* sources_h);
Table buildExponentialTable(void);

// Offload function to execute on the device (GPU)
#pragma offload target(mic:0) \
  in(SA_h:Source_Arrays*)   \
  out(SA_d:Source_Arrays*)  \
  in(sources_h:Source*)
void initialize_device_sources_offload(Input I, Source_Arrays* SA_h, Source_Arrays* SA_d, Source* sources_h);

// Host function to execute on the host (CPU)
void initialize_host_sources(Input I, Source_Arrays* SA_h, Source* sources_h);

#endif  // _SIMPLE_MOC_KERNEL_HEADER_H_