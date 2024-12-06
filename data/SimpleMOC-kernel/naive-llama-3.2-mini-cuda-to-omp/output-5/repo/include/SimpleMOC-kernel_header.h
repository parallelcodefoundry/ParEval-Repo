#ifndef SIMPLEMOC_KERNEL_HEADER_H
#define SIMPLEMOC_KERNEL_HEADER_H

#include <omp.h>
#include <stdio.h>

// Structure to hold source array data
struct Source {
    int id;
    float fine_source_value;
    float fine_flux_value;
    float SigT_value;
};

// Structure to hold table of exponential values for linear interpolation
struct Table {
    float dx;
    float maxVal;
    int N;
    float values[2*N];
};

// Function prototypes
extern Source* initialize_device_sources(Input input, Source_Arrays source_arrays_h, Source_Arrays source_arrays_d, Source* source_arrays_h_data);
extern Source* initialize_device_sources_fully(Input input, Source_Arrays source_arrays_h, Source_Arrays source_arrays_d, Source* source_arrays_h_data);
extern Table buildExponentialTable(void);

// Data structure to hold global variables
typedef struct {
    Input input;
    Source* source_arrays_h_data;
    Source* source_arrays_d_data;
    Source_Arrays source_arrays_h;
    Source_Arrays source_arrays_d;
} SimpleMOCKernel;

#endif // SIMPLEMOC_KERNEL_HEADER_H