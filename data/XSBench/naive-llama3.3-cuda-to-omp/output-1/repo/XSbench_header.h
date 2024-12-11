#ifndef __XSBENCH_HEADER_H__
#define __XSBENCH_HEADER_H__

// Header for shared utilities across XSBench versions

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <omp.h>
#include <stdint.h>

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
    int* num_nucs;                     // Length = length_num_nucs;
    double* concs;                     // Length = length_concs
    int* mats;                         // Length = length_mats
    double* unionized_energy_array;    // Length = length_unionized_energy_array
    int* index_grid;                   // Length = length_index_grid
    NuclideGridPoint* nuclide_grid;    // Length = length_nuclide_grid
    int length_num_nucs;
    int length_concs;
    int length_mats;
    int length_unionized_energy_array;
    long length_index_grid;
    int length_nuclide_grid;
    int max_num_nucs;
    unsigned long* verification;
    int length_verification;
    double* p_energy_samples;
    int length_p_energy_samples;
    int* mat_samples;
    int length_mat_samples;
} SimulationData;

// io.cu
void logo(int version);
void center_print(const char* s, int width);
void border_print(void);
void fancy_int(long a);

typedef struct {
    int nthreads;
    long n_isotopes;
    long n_gridpoints;
    int lookups;
    char* HM;
    int grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
    int hash_bins;
    int particles;
    int simulation_method;
    int binary_mode;
    int kernel_id;
    int num_iterations;
    int num_warmups;
    char* filename;
} Inputs;

typedef struct {
    double device_to_host_time;
    double kernel_time;
    double host_to_device_time;
} Profile;

inline void print_profile(Profile profile, Inputs in) {
    if (in.filename) {
        FILE* output = fopen(in.filename, "w");
        fprintf(output, "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
        fprintf(output, "%f,%f,%f,%d,%d\n",
                profile.host_to_device_time * 1000,
                profile.kernel_time * 1000,
                profile.device_to_host_time * 1000,
                in.num_iterations,
                in.num_warmups);
        fclose(output);
    } else {
        printf("host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n");
        printf("%f,%f,%f,%d,%d\n",
               profile.host_to_device_time * 1000,
               profile.kernel_time * 1000,
               profile.device_to_host_time * 1000,
               in.num_iterations,
               in.num_warmups);
    }
}

// Simulation.cu
void calculate_micro_xs(double p_energy, int nuc, long n_isotopes,
                        long n_gridpoints,
                        double* __restrict__ egrid, int* __restrict__ index_data,
                        NuclideGridPoint* __restrict__ nuclide_grids,
                        long idx, double* __restrict__ xs_vector, int grid_type, int hash_bins);

void calculate_macro_xs(double p_energy, int mat, long n_isotopes,
                         long n_gridpoints, int* __restrict__ num_nucs,
                         double* __restrict__ concs,
                         double* __restrict__ egrid, int* __restrict__ index_data,
                         NuclideGridPoint* __restrict__ nuclide_grids,
                         int* __restrict__ mats,
                         double* __restrict__ macro_xs_vector, int grid_type, int hash_bins, int max_num_nucs);

long grid_search(long n, double quarry, double* __restrict__ A);

long grid_search_nuclide(long n, double quarry, NuclideGridPoint* A, long low, long high);

int pick_mat(uint64_t* seed);

double LCG_random_double(uint64_t* seed);

uint64_t fast_forward_LCG(uint64_t seed, uint64_t n);

// GridInit.cu
SimulationData grid_init_do_not_profile(Inputs in, int mype);
void release_memory(SimulationData SD);

// XSutils.cu
int NGP_compare(const void* a, const void* b);
int double_compare(const void* a, const void* b);
double rn_v(void);
size_t estimate_mem_usage(Inputs in);
double get_time(void);

// Materials.cu
int* load_num_nucs(long n_isotopes);
int* load_mats(int* num_nucs, long n_isotopes, int* max_num_nucs);
double* load_concs(int* num_nucs, int max_num_nucs);

#endif