#ifndef __SimpleMOC_HEADER
#define __SimpleMOC_HEADER

#include <Kokkos_Core.hpp>
#include <Kokkos_Random.hpp>
#include <iostream>
#include <cmath>
#include <string>
#include <ctime>
#include <cassert>
#include <limits>


// User inputs
struct Input {
    int source_2D_regions;
    int source_3D_regions;
    int coarse_axial_intervals;
    int fine_axial_intervals;
    int decomp_assemblies_ax; // Number of subdomains per assembly axially
    long segments;
    int egroups;
    int nthreads; //This will likely be handled by Kokkos
    int streams; //This will likely be handled by Kokkos
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
Kokkos::View<float*,Kokkos::HostSpace> buildExponentialTable();
Input set_default_input();
double mem_estimate(Input I);

// Kernel functions (Kokkos::parallel_for)
void run_kernel(const Input& I, const Source_Arrays& SA, const Kokkos::View<Source*>& S, const Table& table,
                 Kokkos::View<float*>& state_fluxes, int N_state_fluxes);

void interpolateTable(const Table& table, float x, float* out);


// IO functions (remain largely unchanged)
void logo(int version);
void center_print(const char* s, int width);
void border_print();
void fancy_int(int a);
void print_input_summary(Input input);
void read_CLI(int argc, char* argv[], Input* input);
void print_CLI_error();


#endif