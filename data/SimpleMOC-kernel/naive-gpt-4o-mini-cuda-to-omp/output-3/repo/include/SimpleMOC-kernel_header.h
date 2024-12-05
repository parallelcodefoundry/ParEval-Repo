#ifndef __SimpleMOC_header
#define __SimpleMOC_header

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>

#define OMP_ERROR_CHECK

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
    size_t nbytes;
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

// Function prototypes
void run_kernel(Input I, Source *S, Source_Arrays SA, Table *table, float *state_fluxes, int N_state_fluxes);
void interpolateTable(Table *table, float x, float *out);
void setup_kernel(Input I);
void init_flux_states(float *flux_states, int N_flux_states, Input I);
Source *initialize_sources(Input I, Source_Arrays *SA);
Source *initialize_device_sources(Input I, Source_Arrays *SA_h, Source_Arrays *SA_d, Source *sources_h);
Table buildExponentialTable(void);
Input set_default_input(void);
double mem_estimate(Input I);

// Error handling
void __ompCheckError(const char *file, const int line);

#endif