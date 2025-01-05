#ifndef __XSBENCH_HEADER_HPP__
#define __XSBENCH_HEADER_HPP__

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <Kokkos_Core.hpp>
#include <stdint.h>
#include <string>
#include <chrono>
#include "XSbench_shared_header.h"

// Grid types
enum GridType {
  UNIONIZED = 0,
  NUCLIDE = 1,
  HASH = 2
};

// Simulation types
enum SimulationMethod {
  HISTORY_BASED = 1,
  EVENT_BASED = 2
};

// Binary Mode Type
enum BinaryMode {
  NONE = 0,
  READ = 1,
  WRITE = 2
};

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
  Kokkos::View<int*> num_nucs;
  Kokkos::View<double*> concs;
  Kokkos::View<int*> mats;
  Kokkos::View<double*> unionized_energy_array;
  Kokkos::View<int*> index_grid;
  Kokkos::View<NuclideGridPoint*> nuclide_grid;
  int max_num_nucs;
  Kokkos::View<unsigned long*> verification;
  Kokkos::View<double*> p_energy_samples;
  Kokkos::View<int*> mat_samples;

};

// io.cpp
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

// Simulation.cpp
unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile);
KOKKOS_FUNCTION void calculate_micro_xs(double p_energy, int nuc, long n_isotopes,
                                        long n_gridpoints,
                                        const Kokkos::View<double*>::const_type& egrid, const Kokkos::View<int*>::const_type& index_data,
                                        const Kokkos::View<NuclideGridPoint*>::const_type& nuclide_grids,
                                        long idx, Kokkos::View<double*> xs_vector, GridType grid_type, int hash_bins);
KOKKOS_FUNCTION void calculate_macro_xs(double p_energy, int mat, long n_isotopes,
                                        long n_gridpoints, const Kokkos::View<int*>::const_type& num_nucs,
                                        const Kokkos::View<double*>::const_type& concs,
                                        const Kokkos::View<double*>::const_type& egrid, const Kokkos::View<int*>::const_type& index_data,
                                        const Kokkos::View<NuclideGridPoint*>::const_type& nuclide_grids,
                                        const Kokkos::View<int*>::const_type& mats,
                                        Kokkos::View<double*> macro_xs_vector, GridType grid_type, int hash_bins, int max_num_nucs);
KOKKOS_FUNCTION long grid_search(long n, double quarry, const Kokkos::View<double*>::const_type& A);
KOKKOS_FUNCTION long grid_search_nuclide(long n, double quarry, const Kokkos::View<NuclideGridPoint*>::const_type& A, long low, long high);
KOKKOS_FUNCTION int pick_mat(uint64_t* seed);
KOKKOS_FUNCTION double LCG_random_double(uint64_t* seed);
KOKKOS_FUNCTION uint64_t fast_forward_LCG(uint64_t seed, uint64_t n);

unsigned long long run_event_based_simulation_optimization_1(Inputs in, SimulationData GSD, int mype);
unsigned long long run_event_based_simulation_optimization_2(Inputs in, SimulationData GSD, int mype);
unsigned long long run_event_based_simulation_optimization_3(Inputs in, SimulationData GSD, int mype);
unsigned long long run_event_based_simulation_optimization_4(Inputs in, SimulationData GSD, int mype);
unsigned long long run_event_based_simulation_optimization_5(Inputs in, SimulationData GSD, int mype);
unsigned long long run_event_based_simulation_optimization_6(Inputs in, SimulationData GSD, int mype);

// GridInit.cpp
SimulationData grid_init_do_not_profile(Inputs in, int mype);
void release_memory(SimulationData SD);

// XSutils.cpp
int NGP_compare(const void* a, const void* b);
int double_compare(const void* a, const void* b);
double rn_v(void);
size_t estimate_mem_usage(Inputs in);
double get_time(void);

// Materials.cpp
int* load_num_nucs(long n_isotopes);
int* load_mats(int* num_nucs, long n_isotopes, int* max_num_nucs);
double* load_concs(int* num_nucs, int max_num_nucs);

#endif