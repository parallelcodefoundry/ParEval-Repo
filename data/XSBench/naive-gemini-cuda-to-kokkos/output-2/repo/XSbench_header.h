#ifndef __XSBENCH_HEADER_HPP__
#define __XSBENCH_HEADER_HPP__

#include <iostream>
#include <cmath>
#include <cassert>
#include <Kokkos_Core.hpp>
#include <Kokkos_Random.hpp>
#include <algorithm> //for std::sort
#include <stdint.h>
#include <string>
#include "XSbench_shared_header.h"

// Grid types
enum GridType { UNIONIZED, NUCLIDE, HASH };

// Simulation types
enum SimulationMethod { HISTORY_BASED, EVENT_BASED };

// Binary Mode Type
enum BinaryMode { NONE, READ, WRITE };

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
  size_t length_num_nucs;
  size_t length_concs;
  size_t length_mats;
  size_t length_unionized_energy_array;
  size_t length_index_grid;
  size_t length_nuclide_grid;
  int max_num_nucs;
  Kokkos::View<unsigned long*> verification;
  size_t length_verification;
  Kokkos::View<double*> p_energy_samples;
  size_t length_p_energy_samples;
  Kokkos::View<int*> mat_samples;
  size_t length_mat_samples;
};


// io.hpp
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


// Simulation.hpp
unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile);
//Kokkos Kernel declarations (replace __global__ with Kokkos::Functor)
class XSLookupKernelBaseline;
class SamplingKernel;
class XSLookupKernelOptimization1;
class XSLookupKernelOptimization2;
class XSLookupKernelOptimization3;
class XSLookupKernelOptimization4;
class XSLookupKernelOptimization5;

void calculate_micro_xs(double p_energy, int nuc, long n_isotopes,
                        long n_gridpoints,
                        const Kokkos::View<double*>& egrid, const Kokkos::View<int*>& index_data,
                        const Kokkos::View<NuclideGridPoint*>& nuclide_grids,
                        long idx, Kokkos::View<double*> xs_vector, GridType grid_type, int hash_bins);
void calculate_macro_xs(double p_energy, int mat, long n_isotopes,
                        long n_gridpoints, const Kokkos::View<int*>& num_nucs,
                        const Kokkos::View<double*>& concs,
                        const Kokkos::View<double*>& egrid, const Kokkos::View<int*>& index_data,
                        const Kokkos::View<NuclideGridPoint*>& nuclide_grids,
                        const Kokkos::View<int*>& mats,
                        Kokkos::View<double*> macro_xs_vector, GridType grid_type, int hash_bins, int max_num_nucs);
long grid_search(long n, double quarry, const Kokkos::View<double*>& A);
long grid_search_nuclide(long n, double quarry, const Kokkos::View<NuclideGridPoint*>& A, long low, long high);
int pick_mat(Kokkos::Random::LCG64& rng);
double LCG_random_double(Kokkos::Random::LCG64& rng);
uint64_t fast_forward_LCG(uint64_t seed, uint64_t n);


// GridInit.hpp
SimulationData grid_init_do_not_profile(Inputs in, int mype);
//Remove cudaMalloc and cudaMemcpy, use Kokkos::create_mirror_view instead.
SimulationData move_simulation_data_to_device(Inputs in, int mype, SimulationData SD);
void release_device_memory(SimulationData GSD);
void release_memory(SimulationData SD);


// XSutils.hpp
int NGP_compare(const void* a, const void* b);
int double_compare(const void* a, const void* b);
double rn_v(void);
size_t estimate_mem_usage(Inputs in);
double get_time(void);

// Materials.hpp
int* load_num_nucs(long n_isotopes);
int* load_mats(int* num_nucs, long n_isotopes, int* max_num_nucs);
double* load_concs(int* num_nucs, int max_num_nucs);

#endif