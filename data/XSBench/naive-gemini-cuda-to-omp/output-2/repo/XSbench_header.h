#ifndef __XSBENCH_HEADER_H__
#define __XSBENCH_HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <omp.h>
#include <stdint.h>
#include <chrono>
#include "XSbench_shared_header.h"

// Grid types
#define UNIONIZED 0
#define NUCLIDE 1
#define HASH 2

// Simulation types
#define HISTORY_BASED 1
#define EVENT_BASED 2

// Binary Mode Type
#define NONE 0
#define READ 1
#define WRITE 2

// Starting Seed
#define STARTING_SEED 1070


// Structures
typedef struct {
        double energy;
        double total_xs;
        double elastic_xs;
        double absorbtion_xs;
        double fission_xs;
        double nu_fission_xs;
} NuclideGridPoint;

typedef struct {
        int * num_nucs;                     // Length = length_num_nucs;
        double * concs;                     // Length = length_concs
        int * mats;                         // Length = length_mats
        double * unionized_energy_array;    // Length = length_unionized_energy_array
        int * index_grid;                   // Length = length_index_grid
        NuclideGridPoint * nuclide_grid;    // Length = length_nuclide_grid
        int length_num_nucs;
        int length_concs;
        int length_mats;
        int length_unionized_energy_array;
        long length_index_grid;
        int length_nuclide_grid;
        int max_num_nucs;
        unsigned long * verification;
        int length_verification;
        double * p_energy_samples;
        int length_p_energy_samples;
        int * mat_samples;
        int length_mat_samples;
} SimulationData;

// io.c
void logo(int version);
void center_print(const char *s, int width);
void border_print(void);
void fancy_int(long a);
Inputs read_CLI(int argc, char *argv[]);
void print_CLI_error(void);
void print_inputs(Inputs in, int nprocs, int version);
int print_results(Inputs in, int mype, double runtime, int nprocs, unsigned long long vhash);
void binary_write(Inputs in, SimulationData SD);
SimulationData binary_read(Inputs in);

// Simulation.c
unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile);
//The following functions will need to be implemented differently for OpenMP offloading.  They are marked with comments to show where modifications are needed.

//__global__ void xs_lookup_kernel_baseline(Inputs in, SimulationData GSD );
void xs_lookup_kernel_baseline(Inputs in, SimulationData GSD ); // OpenMP kernel

//__device__ void calculate_micro_xs(   double p_energy, int nuc, long n_isotopes, ... );
void calculate_micro_xs(   double p_energy, int nuc, long n_isotopes, long n_gridpoints, double * __restrict__ egrid, int * __restrict__ index_data, NuclideGridPoint * __restrict__ nuclide_grids, long idx, double * __restrict__ xs_vector, int grid_type, int hash_bins ); // OpenMP function

//__device__ void calculate_macro_xs( double p_energy, int mat, long n_isotopes, ... );
void calculate_macro_xs( double p_energy, int mat, long n_isotopes, long n_gridpoints, int * __restrict__ num_nucs, double * __restrict__ concs, double * __restrict__ egrid, int * __restrict__ index_data, NuclideGridPoint * __restrict__ nuclide_grids, int * __restrict__ mats, double * __restrict__ macro_xs_vector, int grid_type, int hash_bins, int max_num_nucs ); // OpenMP function

//__device__ long grid_search( long n, double quarry, double * __restrict__ A);
long grid_search( long n, double quarry, double * __restrict__ A); // OpenMP function

//__host__ __device__ long grid_search_nuclide( long n, double quarry, NuclideGridPoint * A, long low, long high);
long grid_search_nuclide( long n, double quarry, NuclideGridPoint * A, long low, long high); // OpenMP function

//__device__ int pick_mat( uint64_t * seed );
int pick_mat( uint64_t * seed ); // OpenMP function

//__host__ __device__ double LCG_random_double(uint64_t * seed);
double LCG_random_double(uint64_t * seed); // OpenMP function

//__device__ uint64_t fast_forward_LCG(uint64_t seed, uint64_t n);
uint64_t fast_forward_LCG(uint64_t seed, uint64_t n); // OpenMP function


unsigned long long run_event_based_simulation_optimization_1(Inputs in, SimulationData GSD, int mype);
//__global__ void sampling_kernel(Inputs in, SimulationData GSD );
void sampling_kernel(Inputs in, SimulationData GSD); //OpenMP kernel
//__global__ void xs_lookup_kernel_optimization_1(Inputs in, SimulationData GSD );
void xs_lookup_kernel_optimization_1(Inputs in, SimulationData GSD); // OpenMP kernel

unsigned long long run_event_based_simulation_optimization_2(Inputs in, SimulationData GSD, int mype);
//__global__ void xs_lookup_kernel_optimization_2(Inputs in, SimulationData GSD, int m );
void xs_lookup_kernel_optimization_2(Inputs in, SimulationData GSD, int m); // OpenMP kernel

unsigned long long run_event_based_simulation_optimization_3(Inputs in, SimulationData GSD, int mype);
//__global__ void xs_lookup_kernel_optimization_3(Inputs in, SimulationData GSD, int m );
void xs_lookup_kernel_optimization_3(Inputs in, SimulationData GSD, int m); // OpenMP kernel

unsigned long long run_event_based_simulation_optimization_4(Inputs in, SimulationData GSD, int mype);
//__global__ void xs_lookup_kernel_optimization_4(Inputs in, SimulationData GSD, int m, int n_lookups, int offset );
void xs_lookup_kernel_optimization_4(Inputs in, SimulationData GSD, int m, int n_lookups, int offset); // OpenMP kernel

unsigned long long run_event_based_simulation_optimization_5(Inputs in, SimulationData GSD, int mype);
//__global__ void xs_lookup_kernel_optimization_5(Inputs in, SimulationData GSD, int n_lookups, int offset );
void xs_lookup_kernel_optimization_5(Inputs in, SimulationData GSD, int n_lookups, int offset); // OpenMP kernel

unsigned long long run_event_based_simulation_optimization_6(Inputs in, SimulationData GSD, int mype);

// GridInit.c
SimulationData grid_init_do_not_profile(Inputs in, int mype);
SimulationData move_simulation_data_to_device(Inputs in, int mype, SimulationData SD);
void release_device_memory(SimulationData GSD);
void release_memory(SimulationData SD);

// XSutils.c
int NGP_compare(const void *a, const void *b);
int double_compare(const void *a, const void *b);
double rn_v(void);
size_t estimate_mem_usage(Inputs in);
double get_time(void);

// Materials.c
int *load_num_nucs(long n_isotopes);
int *load_mats(int *num_nucs, long n_isotopes, int *max_num_nucs);
double *load_concs(int *num_nucs, int max_num_nucs);

#endif