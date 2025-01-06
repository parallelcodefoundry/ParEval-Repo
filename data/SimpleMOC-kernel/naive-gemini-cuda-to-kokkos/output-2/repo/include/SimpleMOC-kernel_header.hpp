#ifndef __SimpleMOC_HEADER
#define __SimpleMOC_HEADER

#include <Kokkos_Core.hpp>
#include <random>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
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


// Function declarations (forward declarations)
double mem_estimate(Input I);
Table buildExponentialTable();
Input set_default_input();
void __kokkosCheckError(const char* file, const int line);


// Kokkos kernel for run_kernel
KOKKOS_FUNCTION void kokkos_run_kernel(const Input& I, const Source* S,
                                      const Source_Arrays& SA, const Table* table,
                                      Kokkos::View<float*> state_fluxes,
                                      Kokkos::View<int*> state_flux_id,
                                      Kokkos::View<int*> QSR_id,
                                      Kokkos::View<int*> FAI_id,
                                      Kokkos::View<curandState*> state);

// Kokkos kernel for interpolateTable
KOKKOS_FUNCTION void kokkos_interpolateTable(const Table& table, float x, float* out);


// init.cpp
// ... (Implementations of functions declared above) ...

// io.cpp
// ... (Implementations of I/O functions) ...


#endif