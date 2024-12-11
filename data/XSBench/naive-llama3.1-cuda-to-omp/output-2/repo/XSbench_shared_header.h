#ifndef XSBENCH_SHARED_HEADER_H
#define XSBENCH_SHARED_HEADER_H

#include <omp.h>
#include <offload.h>

// Header for shared utilities across XSBench versions

typedef struct{
        int nthreads;
        long n_isotopes;
        long n_gridpoints;
        int lookups;
        char * HM;
        int grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
        int hash_bins;
        int particles;
        int simulation_method;
        int binary_mode;
        int kernel_id;
        int num_iterations;
        int num_warmups;
        char *filename;
} Inputs;

typedef struct{
  double device_to_host_time;
  double kernel_time;
  double host_to_device_time;
} Profile;

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

#pragma offload_attribute(push)
#pragma offload_attribute(allocations:stack)
inline void gpuAssert(offload_err_t code, const char *file, int line, bool abort=true)
{
        if (code != OFFLOAD_SUCCESS) {
                fprintf(stderr,"GPUassert: %s %s %d\n", offloadGetErrorString(code), file, line);
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

// io.cu
void logo(int version);
void center_print(const char *s, int width);
void border_print(void);
void fancy_int(long a);

inline void get_time() {
  #ifdef OPENMP
    return omp_get_wtime();
  #endif

  #ifdef __cplusplus
    // If using C++, we can do this:
    unsigned long us_since_epoch = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
    return (double) us_since_epoch / 1.0e6;
  #else
    struct timeval timecheck;

    gettimeofday(&timecheck, NULL);
    long ms = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

    double time = (double) ms / 1000.0;

    return time;
  #endif
}

// Simulation.cu
unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD);
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

unsigned long long run_event_based_simulation_optimization_1(Inputs in, SimulationData GSD);
__global__ void sampling_kernel(Inputs in, SimulationData GSD );
__global__ void xs_lookup_kernel_optimization_1(Inputs in, SimulationData GSD );

unsigned long long run_event_based_simulation_optimization_2(Inputs in, SimulationData GSD);
__global__ void xs_lookup_kernel_optimization_2(Inputs in, SimulationData GSD, int m );

unsigned long long run_event_based_simulation_optimization_3(Inputs in, SimulationData GSD);
__global__ void xs_lookup_kernel_optimization_3(Inputs in, SimulationData GSD, int m );

unsigned long long run_event_based_simulation_optimization_4(Inputs in, SimulationData GSD);
__global__ void xs_lookup_kernel_optimization_4(Inputs in, SimulationData GSD, int m, int n_lookups, int offset );

unsigned long long run_event_based_simulation_optimization_5(Inputs in, SimulationData GSD);
__global__ void xs_lookup_kernel_optimization_5(Inputs in, SimulationData GSD, int n_lookups, int offset );

unsigned long long run_event_based_simulation_optimization_6(Inputs in, SimulationData GSD);

#pragma offload_attribute(pop)

#endif // XSBENCH_SHARED_HEADER_H