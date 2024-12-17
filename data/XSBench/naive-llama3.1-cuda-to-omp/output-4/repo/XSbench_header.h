#ifndef __XSBENCH_HEADER_H__
#define __XSBENCH_HEADER_H__

#include <omp.h>
#include "openmp_offload.h"
#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<assert.h>
#include<cuda.h>
#include <thrust/reduce.h>
#include <thrust/partition.h>
#include<stdint.h>
#include <chrono>

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

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
        if (code != cudaSuccess)
        {
                fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
                if (abort) exit(code);
        }
}

// Offload function to transfer data from host to device
void offload_data(double* data, int size) {
    #pragma omp target data map(data[:size])
    cudaMemcpyAsync(data, data, size * sizeof(double), cudaMemcpyHostToDevice);
}

// Function to perform macroscopic cross section lookup using OpenMP-offload
__attribute__((target("omp"))) void calculate_macro_xs_offload(
        double p_energy,
        int mat,
        long n_isotopes,
        long n_gridpoints,
        int* num_nucs,
        double* concs,
        double* egrid,
        int* index_data,
        NuclideGridPoint* nuclide_grids,
        int* mats,
        double* macro_xs_vector,
        int grid_type,
        int hash_bins,
        int max_num_nucs
) {
    // Perform macroscopic Cross Section Lookup using OpenMP-offload
    #pragma omp offload target(threads)
    {
        // Allocate memory on device for macro_xs_vector
        double* macro_xs_vector_device;
        #pragma omp target data map(macro_xs_vector_device[:5])
        macro_xs_vector_device = (double*)malloc(5 * sizeof(double));

        // Initialize macro_xs_vector_device with zeros
        memset(macro_xs_vector_device, 0, 5 * sizeof(double));

        // Perform macroscopic Cross Section Lookup
        for (int j = 0; j < num_nucs[mat]; j++) {
            double xs_vector[5];
            p_nuc = mats[mat*max_num_nucs + j];
            conc = concs[mat*max_num_nucs + j];

            // Calculate micro XS for each nuclide
            calculate_micro_xs_offload(
                p_energy,
                p_nuc,
                n_isotopes,
                n_gridpoints,
                egrid,
                index_data,
                nuclide_grids,
                max_idx,
                grid_type,
                hash_bins
            );

            // Add micro XS to macro_xs_vector_device
            for (int k = 0; k < 5; k++) {
                macro_xs_vector_device[k] += xs_vector[k] * conc;
            }
        }

        // Copy macro_xs_vector_device back to host
        offload_data(macro_xs_vector, 5);
    }
}

// Function to perform micro XS calculation using OpenMP-offload
__attribute__((target("omp"))) void calculate_micro_xs_offload(
        double p_energy,
        int nuc,
        long n_isotopes,
        long n_gridpoints,
        double* egrid,
        int* index_data,
        NuclideGridPoint* nuclide_grids,
        int max_idx,
        int grid_type,
        int hash_bins
) {
    // Perform binary search to find energy index in nuclide grid
    int idx = -1;
    if (grid_type == UNIONIZED) {
        idx = grid_search( n_isotopes * n_gridpoints, p_energy, egrid);
    } else if (grid_type == HASH) {
        double du = 1.0 / hash_bins;
        idx = (int)(p_energy / du);
    }

    // Get pointers to nuclide grids and energy index
    NuclideGridPoint* low = &nuclide_grids[nuc*n_gridpoints + idx];
    NuclideGridPoint* high = low + 1;

    // Calculate re-useable interpolation factor
    double f = (high->energy - p_energy) / (high->energy - low->energy);

    // Calculate micro XS
    xs_vector[0] = high->total_xs - f * (high->total_xs - low->total_xs);
    xs_vector[1] = high->elastic_xs - f * (high->elastic_xs - low->elastic_xs);
    xs_vector[2] = high->absorbtion_xs - f * (high->absorbtion_xs - low->absorbtion_xs);
    xs_vector[3] = high->fission_xs - f * (high->fission_xs - low->fission_xs);
    xs_vector[4] = high->nu_fission_xs - f * (high->nu_fission_xs - low->nu_fission_xs);

    // Copy micro XS to macro_xs_vector
    for (int k = 0; k < 5; k++) {
        macro_xs[k] += xs_vector[k];
    }
}

#endif // __XSBENCH_HEADER_H__