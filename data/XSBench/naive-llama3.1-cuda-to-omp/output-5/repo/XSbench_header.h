#ifndef __XSBENCH_HEADER_H__
#define __XSBENCH_HEADER_H__

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<assert.h>
#include<omp.h>

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

#define STARTING_SEED 1070

#include <omp.h>

#pragma omp declare threadprivate(int num_nucs, double concs[m], int mats[m])

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
        double * p_energy_samples;
        int length_p_energy_samples;
        int * mat_samples;
        int length_mat_samples;
} SimulationData;

inline void print_profile(Profile profile, Inputs in) {
  if (in.filename) {
    FILE* output = fopen(in.filename, "w");
    fprintf(output, "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
    fprintf(output, "%f,%f,%f,%d,%d\n",
            profile.host_to_device_time*1000,
            profile.kernel_time*1000,
            profile.device_to_host_time*1000,
            in.num_iterations,
            in.num_warmups);
    fclose(output);
  }
  else {
    printf("host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
    printf("%f,%f,%f,%d,%d\n",
           profile.host_to_device_time*1000,
           profile.kernel_time*1000,
           profile.device_to_host_time*1000,
           in.num_iterations,
           in.num_warmups);
  }
}

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
        double * p_energy_samples;
        int length_p_energy_samples;
        int * mat_samples;
        int length_mat_samples;
} SimulationData;

// io.cu
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

// Simulation.cu
unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile);
__global__ void xs_lookup_kernel_baseline(Inputs in, SimulationData GSD );
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

unsigned long long run_event_based_simulation_optimization_1(Inputs in, SimulationData GSD, int mype);
__global__ void sampling_kernel(Inputs in, SimulationData GSD );
__global__ void xs_lookup_kernel_optimization_1(Inputs in, SimulationData GSD );

unsigned long long run_event_based_simulation_optimization_2(Inputs in, SimulationData GSD, int mype);
__global__ void xs_lookup_kernel_optimization_2(Inputs in, SimulationData GSD, int m );

unsigned long long run_event_based_simulation_optimization_3(Inputs in, SimulationData GSD, int mype);
__global__ void xs_lookup_kernel_optimization_3(Inputs in, SimulationData GSD, int m );

unsigned long long run_event_based_simulation_optimization_4(Inputs in, SimulationData GSD, int mype);
__global__ void xs_lookup_kernel_optimization_4(Inputs in, SimulationData GSD, int m, int n_lookups, int offset );

unsigned long long run_event_based_simulation_optimization_5(Inputs in, SimulationData GSD, int mype);
__global__ void xs_lookup_kernel_optimization_5(Inputs in, SimulationData GSD, int n_lookups, int offset );

unsigned long long run_event_based_simulation_optimization_6(Inputs in, SimulationData GSD, int mype);

// GridInit.cu
SimulationData grid_init_do_not_profile( Inputs in, int mype );
SimulationData move_simulation_data_to_device( Inputs in, int mype, SimulationData SD );
void release_device_memory(SimulationData GSD);
void release_memory(SimulationData SD);

// XSutils.cu
int double_compare(const void * a, const void * b)
{
	double A = *((double *) a);
	double B = *((double *) b);

	if( A > B )
		return 1;
	else if( A < B )
		return -1;
	else
		return 0;
}

int NGP_compare(const void * a, const void * b)
{
	NuclideGridPoint A = *((NuclideGridPoint *) a);
	NuclideGridPoint B = *((NuclideGridPoint *) b);

	if( A.energy > B.energy )
		return 1;
	else if( A.energy < B.energy )
		return -1;
	else
		return 0;
}

// RNG Used for Verification Option.
// This one has a static seed (must be set manually in source).
// Park & Miller Multiplicative Conguential Algorithm
// From "Numerical Recipes" Second Edition
double rn_v(void)
{
	static unsigned long seed = 1337;
	double ret;
	unsigned long n1;
	unsigned long a = 16807;
	unsigned long m = 2147483647;
	n1 = ( a * (seed) ) % m;
	seed = n1;
	ret = (double) n1 / m;
	return ret;
}

size_t estimate_mem_usage( Inputs in )
{
	size_t single_nuclide_grid = in.n_gridpoints * sizeof( NuclideGridPoint );
	size_t all_nuclide_grids   = in.n_isotopes * single_nuclide_grid;
	size_t size_UEG		   = in.n_isotopes*in.n_gridpoints*sizeof(double) + in.n_isotopes*in.n_gridpoints*in.n_isotopes*sizeof(int);
	size_t size_hash_grid	   = in.hash_bins * in.n_isotopes * sizeof(int);
	size_t memtotal;

	if( in.grid_type == UNIONIZED )
		memtotal	  = all_nuclide_grids + size_UEG;
	else if( in.grid_type == NUCLIDE )
		memtotal	  = all_nuclide_grids;
	else
		memtotal	  = all_nuclide_grids + size_hash_grid;

	memtotal	  = ceil(memtotal / (1024.0*1024.0));
	return memtotal;
}

double get_time(void)
{
	#ifdef MPI
	return omp_get_wtime();
	#endif

	#ifdef OPENMP
	return omp_get_wtime();
	#endif

	struct timeval timecheck;

	gettimeofday(&timecheck, NULL);
	long ms = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

	double time = (double) ms / 1000.0;

	return time;
}

// Materials.cu
int * load_num_nucs(long n_isotopes)
{
        int * num_nucs = (int*)malloc(12*sizeof(int));

        // Material 0 is a special case (fuel). The H-M small reactor uses
        // 34 nuclides, while H-M larges uses 300.
        if( n_isotopes == 68 )
                num_nucs[0]  = 34; // HM Small is 34, H-M Large is 321
        else
                num_nucs[0]  = 321; // HM Small is 34, H-M Large is 321

        num_nucs[1]  = 5;
        num_nucs[2]  = 4;
        num_nucs[3]  = 4;
        num_nucs[4]  = 27;
        num_nucs[5]  = 21;
        num_nucs[6]  = 21;
        num_nucs[7]  = 21;
        num_nucs[8]  = 21;
        num_nucs[9]  = 21;
        num_nucs[10] = 9;
        num_nucs[11] = 9;

        return num_nucs;
}

int * load_mats( int * num_nucs, long n_isotopes, int * max_num_nucs )
{
        *max_num_nucs = 0;
        int num_mats = 12;
        for( int m = 0; m < num_mats; m++ )
        {
                if( num_nucs[m] > *max_num_nucs )
                        *max_num_nucs = num_nucs[m];
        }
        int * mats = (int *) malloc( num_mats * (*max_num_nucs) * sizeof(int) );

        // Small H-M has 34 fuel nuclides
        int mats0_Sml[] =  { 58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7,
                8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53, 
                54, 55, 56, 18, 23, 41 }; //fuel
        // Large H-M has 300 fuel nuclides
        int mats0_Lrg[321] =  { 58, 59, 60, 61, 40, 42, 43, 44, 45, 46, 1, 2, 3, 7,
                8, 9, 10, 29, 57, 47, 48, 0, 62, 15, 33, 34, 52, 53,
                54, 55, 56, 18, 23, 41 }; //fuel
        for( int i = 0; i < 321-34; i++ )
                mats0_Lrg[34+i] = 68 + i; // H-M large adds nuclides to fuel only

        // These are the non-fuel materials	
        int mats1[] =  { 63, 64, 65, 66, 67 }; // cladding
        int mats2[] =  { 24, 41, 4, 5 }; // cold borated water
        int mats3[] =  { 24, 41, 4, 5 }; // hot borated water
        int mats4[] =  { 19, 20, 21, 22, 35, 36, 37, 38, 39, 25, 27, 28, 29,
                30, 31, 32, 26, 49, 50, 51, 11, 12, 13, 14, 6, 16,
        17 }; // RPV
        int mats5[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                49, 50, 51, 11, 12, 13, 14 }; // lower radial reflector
        int mats6[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                49, 50, 51, 11, 12, 13, 14 }; // top reflector / plate
        int mats7[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                49, 50, 51, 11, 12, 13, 14 }; // bottom plate
        int mats8[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                49, 50, 51, 11, 12, 13, 14 }; // bottom nozzle
        int mats9[] =  { 24, 41, 4, 5, 19, 20, 21, 22, 35, 36, 37, 38, 39, 25,
                49, 50, 51, 11, 12, 13, 14 }; // top nozzle
        int mats10[] = { 24, 41, 4, 5, 63, 64, 65, 66, 67 }; // top of FA's
        int mats11[] = { 24, 41, 4, 5, 63, 64, 65, 66, 67 }; // bottom FA's

        // H-M large v small dependency
        if( n_isotopes == 68 )
                memcpy( mats,  mats0_Sml,  num_nucs[0]  * sizeof(int) );	
        else
                memcpy( mats,  mats0_Lrg,  num_nucs[0]  * sizeof(int) );

        // Copy other materials
        memcpy( mats + *max_num_nucs * 1,  mats1,  num_nucs[1]  * sizeof(int) );	
        memcpy( mats + *max_num_nucs * 2,  mats2,  num_nucs[2]  * sizeof(int) );	
        memcpy( mats + *max_num_nucs * 3,  mats3,  num_nucs[3]  * sizeof(int) );	
        memcpy( mats + *max_num_nucs * 4,  mats4,  num_nucs[4]  * sizeof(int) );	
        memcpy( mats + *max_num_nucs * 5,  mats5,  num_nucs[5]  * sizeof(int) );	
        memcpy( mats + *max_num_nucs * 6,  mats6,  num_nucs[6]  * sizeof(int) );	
        memcpy( mats + *max_num_nucs * 7,  mats7,  num_nucs[7]  * sizeof(int) );	
        memcpy( mats + *max_num_nucs * 8,  mats8,  num_nucs[8]  * sizeof(int) );	
        memcpy( mats + *max_num_nucs * 9,  mats9,  num_nucs[9]  * sizeof(int) );	
        memcpy( mats + *max_num_nucs * 10, mats10, num_nucs[10] * sizeof(int) );	
        memcpy( mats + *max_num_nucs * 11, mats11, num_nucs[11] * sizeof(int) );	

        return mats;
}

double * load_concs( int * num_nucs, int max_num_nucs )
{
        uint64_t seed = STARTING_SEED * STARTING_SEED;
        double * concs = (double *) malloc( 12 * max_num_nucs * sizeof( double ) );

        for( int i = 0; i < 12; i++ )
                for( int j = 0; j < num_nucs[i]; j++ )
                        concs[i * max_num_nucs + j] = LCG_random_double(&seed);

        // test
        /*
    for( int i = 0; i < 12; i++ )
        for( int j = 0; j < num_nucs[i]; j++ )
            printf("concs[%d][%d] = %lf\n", i, j, concs[i][j] );
        */

        return concs;
}
#endif