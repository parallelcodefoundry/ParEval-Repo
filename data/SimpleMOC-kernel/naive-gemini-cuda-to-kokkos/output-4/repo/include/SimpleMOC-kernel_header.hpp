#ifndef __SimpleMOC_HEADER
#define __SimpleMOC_HEADER

#include <Kokkos_Core.hpp>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <assert.h>


// User inputs
struct Input {
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
};

// Source Region Structure
struct Source {
    long fine_flux_id;
    long fine_source_id;
    long sigT_id;
};

// Source Arrays
struct Source_Arrays {
    Kokkos::View<float*> fine_flux_arr;
    Kokkos::View<float*> fine_source_arr;
    Kokkos::View<float*> sigT_arr;
};

// Table structure for computing exponential
struct Table {
    Kokkos::View<float*> values;
    float dx;
    float maxVal;
    int N;
};


// Function declarations (replace .cu with .hpp/.cpp as appropriate)
void run_kernel(const Input& I, const Source_Arrays& SA, const Kokkos::View<Source*>& S, const Table& table, Kokkos::View<float*>& state_fluxes);
void interpolateTable(const Table& table, float x, float* out);
double mem_estimate(const Input& I);
void setup_kernel(Kokkos::View<Kokkos::rand::XorShift64Star>& rng_states, const Input& I);
void init_flux_states(Kokkos::View<float*>& flux_states, int N_flux_states, const Input& I, Kokkos::View<Kokkos::rand::XorShift64Star>& rng_states);
Kokkos::View<Source*> initialize_sources(const Input& I, Source_Arrays& SA);
Source_Arrays initialize_device_sources(const Input& I, const Source_Arrays& SA_h);
Table buildExponentialTable(void);
Input set_default_input(void);

// io.cpp
void logo(int version);
void center_print(const char* s, int width);
void border_print(void);
void fancy_int(int a);
void print_input_summary(const Input& input);
void read_CLI(int argc, char* argv[], Input* input);
void print_CLI_error(void);

#endif