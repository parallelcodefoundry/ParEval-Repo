#include "XSbench_shared_header.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

// Comparison function for sorting NuclideGridPoint structures
int NGP_compare(const void *a, const void *b) {
    NuclideGridPoint A = *((NuclideGridPoint *)a);
    NuclideGridPoint B = *((NuclideGridPoint *)b);

    if (A.energy > B.energy)
        return 1;
    else if (A.energy < B.energy)
        return -1;
    else
        return 0;
}

// Comparison function for sorting double values
int double_compare(const void *a, const void *b) {
    double A = *((double *)a);
    double B = *((double *)b);

    if (A > B)
        return 1;
    else if (A < B)
        return -1;
    else
        return 0;
}

// RNG used for verification option
double rn_v() {
    static unsigned long seed = 1337;
    double ret;
    unsigned long n1;
    unsigned long a = 16807;
    unsigned long m = 2147483647;
    n1 = (a * (seed)) % m;
    seed = n1;
    ret = (double)n1 / m;
    return ret;
}

// Estimates memory usage
size_t estimate_mem_usage(Inputs in) {
    size_t single_nuclide_grid = in.n_gridpoints * sizeof(NuclideGridPoint);
    size_t all_nuclide_grids = in.n_isotopes * single_nuclide_grid;
    size_t size_UEG = in.n_isotopes * in.n_gridpoints * sizeof(double) + in.n_isotopes * in.n_gridpoints * in.n_isotopes * sizeof(int);
    size_t size_hash_grid = in.hash_bins * in.n_isotopes * sizeof(int);
    size_t memtotal;

    if (in.grid_type == UNIONIZED)
        memtotal = all_nuclide_grids + size_UEG;
    else if (in.grid_type == NUCLIDE)
        memtotal = all_nuclide_grids;
    else
        memtotal = all_nuclide_grids + size_hash_grid;

    memtotal = ceil(memtotal / (1024.0 * 1024.0));
    return memtotal;
}

// Gets the current time
double get_time() {
#ifdef _OPENMP
    return omp_get_wtime();
#endif

#ifdef __cplusplus
    // If using C++, we can do this:
    unsigned long us_since_epoch = std::chrono::high_resolution_clock::now().time_since_epoch() / std::chrono::microseconds(1);
    return (double)us_since_epoch / 1.0e6;
#else
    struct timeval timecheck;

    gettimeofday(&timecheck, NULL);
    long ms = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

    double time = (double)ms / 1000.0;

    return time;
#endif
}