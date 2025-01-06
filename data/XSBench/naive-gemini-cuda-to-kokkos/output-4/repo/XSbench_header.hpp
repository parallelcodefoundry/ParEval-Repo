#ifndef __XSBENCH_HEADER_HPP__
#define __XSBENCH_HEADER_HPP__

#include <iostream>
#include <cmath>
#include <cassert>
#include <Kokkos_Core.hpp>
#include <limits> // Required for numeric_limits
#include <stdint.h>
#include <chrono>
#include "XSbench_shared_header.h"

// Grid types
enum class GridType { Unionized, Nuclide, Hash };

// Simulation types
enum class SimulationMethod { HistoryBased, EventBased };

// Binary Mode Type
enum class BinaryMode { None, Read, Write };

// Starting Seed
constexpr uint64_t STARTING_SEED = 1070;


// Structures
struct NuclideGridPoint {
        double energy;
        double total_xs;
        double elastic_xs;
        double absorbtion_xs;
        double fission_xs;
        double nu_fission_xs;
};

struct SimulationData {
        Kokkos::View<int*> num_nucs;                     // Length = length_num_nucs;
        Kokkos::View<double*> concs;                     // Length = length_concs
        Kokkos::View<int*> mats;                         // Length = length_mats
        Kokkos::View<double*> unionized_energy_array;    // Length = length_unionized_energy_array
        Kokkos::View<int*> index_grid;                   // Length = length_index_grid
        Kokkos::View<NuclideGridPoint*> nuclide_grid;    // Length = length_nuclide_grid
        int length_num_nucs;
        int length_concs;
        int length_mats;
        int length_unionized_energy_array;
        long length_index_grid;
        int length_nuclide_grid;
        int max_num_nucs;
        Kokkos::View<unsigned long*> verification;
        int length_verification;
        Kokkos::View<double*> p_energy_samples;
        int length_p_energy_samples;
        Kokkos::View<int*> mat_samples;
        int length_mat_samples;
};

// Forward declarations for functions in other files
void logo(int version);
void center_print(const char* s, int width);
void border_print(void);
void fancy_int(long a);
Inputs read_CLI(int argc, char* argv[]);
void print_CLI_error(void);
void print_inputs(Inputs in, int nprocs, int version);
int print_results(Inputs in, int mype, double runtime, int nprocs, unsigned long long vhash);
void binary_write(Inputs in, SimulationData SD);
SimulationData binary_read(Inputs in);


//Kokkos Kernel functions (replace CUDA kernels)
KOKKOS_FUNCTION
long grid_search_nuclide(long n, double quarry, const NuclideGridPoint* A, long low, long high) {
    long lowerLimit = low;
    long upperLimit = high;
    long examinationPoint;
    long length = upperLimit - lowerLimit;

    while (length > 1) {
        examinationPoint = lowerLimit + (length / 2);

        if (A[examinationPoint].energy > quarry)
            upperLimit = examinationPoint;
        else
            lowerLimit = examinationPoint;

        length = upperLimit - lowerLimit;
    }

    return lowerLimit;
}

KOKKOS_FUNCTION
long grid_search(long n, double quarry, const double* A) {
    long lowerLimit = 0;
    long upperLimit = n - 1;
    long examinationPoint;
    long length = upperLimit - lowerLimit;

    while (length > 1) {
        examinationPoint = lowerLimit + (length / 2);

        if (A[examinationPoint] > quarry)
            upperLimit = examinationPoint;
        else
            lowerLimit = examinationPoint;

        length = upperLimit - lowerLimit;
    }

    return lowerLimit;
}

KOKKOS_FUNCTION
int pick_mat(uint64_t* seed) {
    // Probabilistic distribution of materials
    double dist[12] = {0.140, 0.052, 0.275, 0.134, 0.154, 0.064, 0.066, 0.055, 0.008, 0.015, 0.025, 0.013};
    double roll = LCG_random_double(seed);

    double running_sum = 0.0;
    for (int i = 0; i < 12; ++i) {
        running_sum += dist[i];
        if (roll < running_sum) return i;
    }
    return 0; //Should not reach here.
}

KOKKOS_FUNCTION
double LCG_random_double(uint64_t* seed) {
    // LCG parameters
    constexpr uint64_t m = 9223372036854775808ULL; // 2^63
    constexpr uint64_t a = 2806196910506780709ULL;
    constexpr uint64_t c = 1ULL;
    *seed = (a * (*seed) + c) % m;
    return (double)(*seed) / (double)m;
}

KOKKOS_FUNCTION
uint64_t fast_forward_LCG(uint64_t seed, uint64_t n) {
    // LCG parameters
    constexpr uint64_t m = 9223372036854775808ULL; // 2^63
    constexpr uint64_t a = 2806196910506780709ULL;
    constexpr uint64_t c = 1ULL;

    n = n % m;

    uint64_t a_new = 1;
    uint64_t c_new = 0;

    while (n > 0) {
        if (n & 1) {
            a_new *= a;
            c_new = c_new * a + c;
        }
        c *= (a + 1);
        a *= a;
        n >>= 1;
    }

    return (a_new * seed + c_new) % m;
}


//Other Function Declarations (Simulation.cpp, GridInit.cpp, XSutils.cpp, Materials.cpp)
unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData& SD, int mype, Profile* profile);
unsigned long long run_event_based_simulation_optimization_1(Inputs in, SimulationData& SD, int mype);
unsigned long long run_event_based_simulation_optimization_2(Inputs in, SimulationData& SD, int mype);
unsigned long long run_event_based_simulation_optimization_3(Inputs in, SimulationData& SD, int mype);
unsigned long long run_event_based_simulation_optimization_4(Inputs in, SimulationData& SD, int mype);
unsigned long long run_event_based_simulation_optimization_5(Inputs in, SimulationData& SD, int mype);
unsigned long long run_event_based_simulation_optimization_6(Inputs in, SimulationData& SD, int mype);

SimulationData grid_init_do_not_profile(Inputs in, int mype);
SimulationData move_simulation_data_to_device(Inputs in, int mype, SimulationData& SD);
void release_device_memory(SimulationData& SD);
void release_memory(SimulationData& SD);

int NGP_compare(const void* a, const void* b);
int double_compare(const void* a, const void* b);
double rn_v(void);
size_t estimate_mem_usage(Inputs in);
double get_time(void);


int* load_num_nucs(long n_isotopes);
int* load_mats(int* num_nucs, long n_isotopes, int* max_num_nucs);
double* load_concs(int* num_nucs, int max_num_nucs);

#endif