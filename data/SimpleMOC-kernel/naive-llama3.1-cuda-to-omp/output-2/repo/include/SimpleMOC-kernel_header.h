#ifndef __SimpleMOC_header
#define __SimpleMOC_header

#include <omp.h>
#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

// OpenMP Error Handling Macro (not applicable for offloading)
#define CUDA_ERROR_CHECK

// User inputs
typedef struct {
    int source_2D_regions;
    int source_3D_regions;
    int coarse_axial_intervals;
    int fine_axial_intervals;
    int decomp_assemblies_ax; // Number of subdomains per assembly axially
    long segments;
    int egroups;
    int nthreads;
    int streams;
    int seg_per_thread;
} Input;

// Source Region Structure
typedef struct {
    long fine_flux_id;
    long fine_source_id;
    long sigT_id;
} Source;

// Source Arrays
typedef struct {
    float *fine_flux_arr;
    float *fine_source_arr;
    float *sigT_arr;
} Source_Arrays;

// Table structure for computing exponential
typedef struct {
    float values[706];
    float dx;
    float maxVal;
    int N;
} Table;

// Offload kernel functions
extern "C" __attribute__((noinline)) void run_kernel(offload_target(hsa, cl), Input *I, Source **S,
        Source_Arrays *SA, Table *table, curandState *state, float *state_fluxes, int N_state_fluxes);

// Offload kernel functions
extern "C" __attribute__((noinline)) void interpolateTable(offload_target(hsa, cl), Table *table, float x,
        float *out);

#endif // __SimpleMOC_header