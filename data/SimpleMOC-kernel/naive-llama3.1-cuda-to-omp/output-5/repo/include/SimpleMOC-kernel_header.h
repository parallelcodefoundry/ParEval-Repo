#ifndef __SimpleMOC_header
#define __SimpleMOC_header

#include <omp.h>
#include <openacc.h>

// Offload device types (OpenACC)
#define DEVICE_TYPE_HOST 0
#define DEVICE_TYPE_DEVICE 1

// User inputs
typedef struct{
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
typedef struct{
    long fine_flux_id;
    long fine_source_id;
    long sigT_id;
} Source;

// Source Arrays
typedef struct{
    float * fine_flux_arr;
    float * fine_source_arr;
    float * sigT_arr;
} Source_Arrays;

// Table structure for computing exponential
typedef struct{
    float values[706];
    float dx;
    float maxVal;
    int N;
} Table;

// kernel.c
void run_kernel( Input I, Source *  S,
        Source_Arrays SA, Table *  table, float *  state_fluxes, int N_state_fluxes);
__device void interpolateTable(Table *  table, float x, float *  out);

// init.c
double mem_estimate( Input I );
void setup_kernel(curandState *state, Input I);
void	init_flux_states( float * flux_states, int N_flux_states, Input I, curandState * state);
Source * initialize_sources( Input I, Source_Arrays * SA );
Source * initialize_device_sources( Input I, Source_Arrays * SA_h, Source_Arrays * SA_d, Source * sources_h );
Table buildExponentialTable( void );
Input set_default_input( void );

// io.c
void logo(int version);
void center_print(const char *s, int width);
void border_print(void);
void fancy_int( int a );
void print_input_summary(Input input);
void read_CLI( int argc, char * argv[], Input * input );
void print_CLI_error(void);

#endif

#pragma omp declare target(nvptx64-compiler)