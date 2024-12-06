#ifndef SIMPLEMOC_KERNEL_HEADER_H
#define SIMPLEMOC_KERNEL_HEADER_H

#include <omp.h>

// Table structure for exponential values
typedef struct {
    float dx;
    float maxVal;
    int N;
    float* values;
} Table;

// Source array structure
typedef struct {
    float* fine_source_arr;
    float* fine_flux_arr;
    float* sigT_arr;
} Source_Arrays;

// Device source array structure
typedef struct {
    float* fine_source_arr;
    float* fine_flux_arr;
    float* sigT_arr;
    Source* sources_d;
} Source_Arrays_Device;

// Function to allocate memory for device source arrays
Source_Arrays_Device initialize_device_sources( Input I, Source_Arrays SA_h, Source_Arrays SA_d, Source *sources_h );

// Function to build an exponential table of values for linear interpolation
Table buildExponentialTable(void);

#endif  // SIMPLEMOC_KERNEL_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

void translateHeader();

#ifdef __cplusplus
}
#endif