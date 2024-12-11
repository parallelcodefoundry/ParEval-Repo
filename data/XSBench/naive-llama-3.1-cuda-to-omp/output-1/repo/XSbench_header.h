#ifndef __XSBENCH_HEADER_H__
#define __XSBENCH_HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <cuda.h>
#include <omp.h>
#include "omp_offload.h"

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

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
        if (code != cudaSuccess)
        {
                fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
                if (abort) exit(code);
        }
}

// Structures
typedef struct{
        double energy;
        double total_xs;
        double elastic_xs;
        double absorbtion_xs;
        double fission_xs;
        double nu_fission_xs;
} NuclideGridPoint;

typedef struct{
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
} SimulationData;

// Function declarations
void logo(int version);
void center_print(const char *s, int width);
void border_print(void);
void fancy_int(long a);

Inputs read_CLI( int argc, char * argv[] );
void print_CLI_error(void);
void print_inputs(Inputs in, int nprocs, int version);
int print_results( Inputs in, int mype, double runtime, int nprocs, unsigned long long vhash );
void binary_write( Inputs in, SimulationData SD );
SimulationData binary_read( Inputs in );

unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile);
__device__ void calculate_micro_xs(   double p_energy, int nuc, long n_isotopes,
                                   long n_gridpoints,
                                   double * __restrict__ egrid, int * __restrict__ index_data,
                                   NuclideGridPoint * __restrict__ nuclide_grids,
                                   long idx, double * __restrict__ xs_vector, int grid_type, int hash_bins );
__device__ void calculate_macro_xs( double p_energy, int mat, long n_isotopes,
                                   long n_gridpoints, int * __restrict__ num_nucs,
                                   double * __restrict__ concs,
                                   double * __restrict__ egrid, int * __restrict__ index_data,
                                   NuclideGridPoint * __restrict__ nuclide_grids,
                                   int * __restrict__ mats,
                                   double * __restrict__ macro_xs_vector, int grid_type, int hash_bins, int max_num_nucs );
__device__ long grid_search( long n, double quarry, double * __restrict__ A);
__host__ __device__ long grid_search_nuclide( long n, double quarry, NuclideGridPoint * A, long low, long high);
__device__ int pick_mat( uint64_t * seed );
__host__ __device__ double LCG_random_double(uint64_t * seed);
__device__ uint64_t fast_forward_LCG(uint64_t seed, uint64_t n);

unsigned long long run_event_based_simulation_optimization_1(Inputs in, SimulationData SD, int mype);
void sampling_kernel(int in, SimulationData GSD );
void xs_lookup_kernel_optimization_1(int in, SimulationData GSD );

unsigned long long run_event_based_simulation_optimization_2(Inputs in, SimulationData SD, int mype);
void xs_lookup_kernel_optimization_2(int in, SimulationData GSD, int m );

unsigned long long run_event_based_simulation_optimization_3(Inputs in, SimulationData SD, int mype);
void xs_lookup_kernel_optimization_3(int in, SimulationData GSD, int m );

unsigned long long run_event_based_simulation_optimization_4(Inputs in, SimulationData SD, int mype);
void xs_lookup_kernel_optimization_4(int in, SimulationData GSD, int m, int n_lookups, int offset );

unsigned long long run_event_based_simulation_optimization_5(Inputs in, SimulationData SD, int mype);
void xs_lookup_kernel_optimization_5(int in, SimulationData GSD, int n_lookups, int offset );

unsigned long long run_event_based_simulation_optimization_6(Inputs in, SimulationData SD, int mype);

// Offloading functions
__attribute__((reqd_work_group_size(1, 1, 1)))
__kernel void sampling_kernel(int in, SimulationData GSD )
{
    // The lookup ID.
    const int i = get_global_id(0);

    if (i >= in.lookups)
        return;

    // Set the initial seed value
    uint64_t seed = STARTING_SEED;

    // Forward seed to lookup index (we need 2 samples per lookup)
    seed = fast_forward_LCG(seed, 2*i);

    // Randomly pick an energy and material for the particle
    double p_energy = LCG_random_double(&seed);
    int mat         = pick_mat(&seed);

    // Store sample data in state array
    GSD.p_energy_samples[i] = p_energy;
    GSD.mat_samples[i] = mat;
}

__attribute__((reqd_work_group_size(1, 1, 1)))
__kernel void xs_lookup_kernel_optimization_1(int in, SimulationData GSD )
{
    // The lookup ID. Used to set the seed, and to store the verification value
    const int i = get_global_id(0);

    if (i >= in.lookups)
        return;

    double macro_xs_vector[5] = {0};

    // Perform macroscopic Cross Section Lookup
    calculate_macro_xs(
            GSD.p_energy_samples[i],        // Sampled neutron energy (in lethargy)
            GSD.mat_samples[i],             // Sampled material type index neutron is in
            in.n_isotopes,   // Total number of isotopes in simulation
            in.n_gridpoints, // Number of gridpoints per isotope in simulation
            GSD.num_nucs,     // 1-D array with number of nuclides per material
            GSD.concs,        // Flattened 2-D array with concentration of each nuclide in each material
            GSD.unionized_energy_array, // 1-D Unionized energy array
            GSD.index_grid,   // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
            GSD.nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
            GSD.mats,         // Flattened 2-D array with nuclide indices defining composition of each type of material
            macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
            in.grid_type,    // Lookup type (nuclide, hash, or unionized)
            in.hash_bins,    // Number of hash bins used (if using hash lookup type)
            GSD.max_num_nucs  // Maximum number of nuclides present in any material
    );

    // For verification, and to prevent the compiler from optimizing
    // all work out, we interrogate the returned macro_xs_vector array
    // to find its maximum value index, then increment the verification
    // value by that index. In this implementation, we have each thread
    // write to its thread_id index in an array, which we will reduce
    // with a thrust reduction kernel after the main simulation kernel.
    double max = -1.0;
    int max_idx = 0;
    for(int j = 0; j < 5; j++ )
    {
        if( macro_xs_vector[j] > max )
        {
            max = macro_xs_vector[j];
            max_idx = j;
        }
    }
    GSD.verification[i] = max_idx+1;
}

// Other offloading functions...