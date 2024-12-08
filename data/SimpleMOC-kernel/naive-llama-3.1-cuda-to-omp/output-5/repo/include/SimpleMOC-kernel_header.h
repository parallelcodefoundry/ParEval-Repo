#ifndef SIMPLE_MOC_KERNEL_HEADER_H
#define SIMPLE_MOC_KERNEL_HEADER_H

#include <omp.h>

// Define a struct for Source_Arrays
typedef struct {
  float *fine_source_arr;
  float *fine_flux_arr;
  float *sigT_arr;
} Source_Arrays;

// Define a function to build an exponential table
Table buildExponentialTable() {
#pragma offload target(mic:0) map(alloc) arg(table)
  Table table;
  // ... rest of the function remains the same ...
  return table;
}

// Define a function to initialize device sources
Source *initialize_device_sources(Input I, Source_Arrays *SA_h, Source_Arrays *SA_d, Source *sources_h) {
#pragma offload target(mic:0) map(alloc) arg(sources_d)
  // ... rest of the function remains the same ...
  return sources_d;
}

// Define a function to check CUDA errors
void __cudaCheckError(const char *file, const int line) {
#ifdef CUDA_ERROR_CHECK
  // ... rest of the function remains the same ...
#endif
}

// Define a function to initialize device sources from host sources
Source *initialize_device_sources_from_host(Input I, Source_Arrays *SA_h, Source_Arrays *SA_d, Source *sources_h) {
#pragma offload target(mic:0) map(alloc) arg(sources_d)
  // ... rest of the function remains the same ...
  return sources_d;
}

#endif // SIMPLE_MOC_KERNEL_HEADER_H