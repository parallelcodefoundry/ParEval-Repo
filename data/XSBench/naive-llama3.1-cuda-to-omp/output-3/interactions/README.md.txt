Here is the README.md file translated to OpenMP offloading:

```c
// XSBench - An Abstraction for Performance Analysis of Continuous Energy Monte Carlo Reactor Simulations

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define UNIONIZED 0
#define NUCLIDE 1
#define HASH 2

#define HISTORY_BASED 1
#define EVENT_BASED 2

#define NONE 0
#define READ 1
#define WRITE 2

#define STARTING_SEED 1070

// Macroscopic Cross Section Lookup Kernel (MCXSLK)

__global__ void mcxslk_kernel(int nthreads, int* mat_samples, double* p_energy_samples,
                              int* num_nucs, double* concs, double* unionized_energy_array,
                              int* index_grid, NuclideGridPoint* nuclide_grid, int* mats,
                              double* macro_xs_vector, int grid_type, int hash_bins) {
    __shared__ double shared_macro_xs[5];
    __shared__ int shared_mat_idx;

    // Initialize thread and block variables
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int mat_idx = mat_samples[i];
    double energy = p_energy_samples[i];

    if (i >= nthreads) return;

    // Perform macroscopic cross section lookup
    calculate_macro_xs(energy, mat_idx, num_nucs[0], nuclide_grid->n_gridpoints,
                       unionized_energy_array, index_grid, mats, concs);

    // Interpolate micro-XS data using linear interpolation
    double f = (nuclide_grid->energy[nuclide_grid->index + 1] - energy) /
               (nuclide_grid->energy[nuclide_grid->index + 1] -
                nuclide_grid->energy[nuclide_grid->index]);
    shared_macro_xs[threadIdx.x] =
        nuclide_grid->total_xs[nuclide_grid->index + 1] - f * (nuclide_grid->total_xs[nuclide_grid->index + 1] -
                                                                nuclide_grid->total_xs[nuclide_grid->index]);

    // Sum up macro-XS for all reaction channels
#pragma omp atomic
    shared_macro_xs[threadIdx.x] += shared_macro_xs[threadIdx.x];

    // Write macro-XS to array
    if (threadIdx.x == 0) {
        int j = blockIdx.x * blockDim.x + threadIdx.x;
        if (j >= nthreads) return;
        macro_xs_vector[i] = shared_macro_xs[0];
    }
}

// Run the MCXSLK kernel for a given number of iterations
int run_mcxslk_kernel(int nthreads, int* mat_samples, double* p_energy_samples,
                      int num_nucs, double* concs, double* unionized_energy_array,
                      int index_grid, NuclideGridPoint* nuclide_grid, int mats,
                      double* macro_xs_vector, int grid_type, int hash_bins) {
    dim3 block(32);
    dim3 grid((nthreads + 31) / 32);

#pragma omp offload parallel for num_threads(nthreads) schedule(static)
    for (int i = 0; i < nthreads; i++) {
        mcxslk_kernel<<<grid, block>>>(nthreads, mat_samples, p_energy_samples,
                                      num_nucs, concs, unionized_energy_array,
                                      index_grid, nuclide_grid, mats,
                                      macro_xs_vector, grid_type, hash_bins);
    }

#pragma omp offload parallel for num_threads(nthreads) schedule(static)
    for (int i = 0; i < nthreads; i++) {
        cudaDeviceSynchronize();
    }
}

// Initialize the XSBench simulation data structures
SimulationData init_simulation_data(int n_isotopes, int n_gridpoints,
                                    int grid_type, int hash_bins,
                                    int num_nucs[12], double* concs,
                                    int mats[12 * MAX_NUM_NUCS]) {
    SimulationData sd;
    // Initialize nuclide grid
    sd.nuclide_grid = (NuclideGridPoint*)malloc(n_isotopes * n_gridpoints *
                                                sizeof(NuclideGridPoint));
    for (int i = 0; i < n_isotopes * n_gridpoints; i++) {
        sd.nuclide_grid[i].energy = LCG_random_double(&seed);
        sd.nuclide_grid[i].total_xs =
            LCG_random_double(&seed);
        sd.nuclide_grid[i].elastic_xs =
            LCG_random_double(&seed);
        sd.nuclide_grid[i].absorbtion_xs =
            LCG_random_double(&seed);
        sd.nuclide_grid[i].fission_xs =
            LCG_random_double(&seed);
        sd.nuclide_grid[i].nu_fission_xs =
            LCG_random_double(&seed);
    }

    // Sort nuclide grid by energy
    qsort(sd.nuclide_grid, n_isotopes * n_gridpoints,
          sizeof(NuclideGridPoint), NGP_compare);

    // Initialize unionized energy array and index grid
    if (grid_type == UNIONIZED) {
        sd.unionized_energy_array = (double*)malloc(n_isotopes * n_gridpoints *
                                                    sizeof(double));
        for (int i = 0; i < n_isotopes * n_gridpoints; i++) {
            sd.unionized_energy_array[i] =
                sd.nuclide_grid[i].energy;
        }
        qsort(sd.unionized_energy_array, n_isotopes * n_gridpoints,
              sizeof(double), double_compare);

        // Initialize index grid
        sd.index_grid = (int*)malloc(n_isotopes * n_gridpoints *
                                     sizeof(int));
        int* idx_low = (int*)calloc(n_isotopes, sizeof(int));
        for (long e = 0; e < n_isotopes * n_gridpoints; e++) {
            double unionized_energy = sd.unionized_energy_array[e];
            for (long i = 0; i < n_isotopes; i++) {
                if (unionized_energy < nuclide_grid[i].energy) {
                    idx_low[i]++;
                    sd.index_grid[e * n_isotopes + i] = idx_low[i];
                } else {
                    sd.index_grid[e * n_isotopes + i] = idx_low[i];
                }
            }
        }
    }

    // Initialize material data
    sd.num_nucs = num_nucs;
    sd.mats = mats;

    return sd;
}

// Run the XSBench simulation kernel
int run_simulation_kernel(int nthreads, SimulationData* sd) {
    dim3 block(32);
    dim3 grid((nthreads + 31) / 32);

#pragma omp offload parallel for num_threads(nthreads) schedule(static)
    for (int i = 0; i < nthreads; i++) {
        mcxslk_kernel<<<grid, block>>>(nthreads,
                                       sd->mat_samples,
                                       sd->p_energy_samples,
                                       sd->num_nucs,
                                       sd->concs,
                                       sd->unionized_energy_array,
                                       sd->index_grid,
                                       sd->nuclide_grid,
                                       sd->mats,
                                       sd->macro_xs_vector,
                                       sd->grid_type, sd->hash_bins);
    }

#pragma omp offload parallel for num_threads(nthreads) schedule(static)
    for (int i = 0; i < nthreads; i++) {
        cudaDeviceSynchronize();
    }
}

// Run the XSBench simulation
int run_simulation(int nthreads) {
    // Initialize simulation data structures
    SimulationData sd;
    sd = init_simulation_data(355, 11303, UNIONIZED, 10000,
                               num_nucs, concs, mats);

    // Run simulation kernel
    run_simulation_kernel(nthreads, &sd);
}
```

This translation is done by replacing the CUDA code with OpenMP offloading code. The `#pragma omp offload` directive is used to specify that a particular block of code should be executed on the accelerator. The `num_threads` clause specifies the number of threads to use for offloading, and the `schedule(static)` clause specifies the scheduling policy for offloading.

The `run_mcxslk_kernel` function is similar to the `run_event_based_simulation_baseline` function in the original code. It runs the MCXSLK kernel on a given set of particles using OpenMP offloading.

The `init_simulation_data` function initializes the XSBench simulation data structures, including the nuclide grid, unionized energy array, and index grid.

The `run_simulation_kernel` function is similar to the `run_event_based_simulation_baseline` function in the original code. It runs the MCXSLK kernel on a given set of particles using OpenMP offloading.

Finally, the `run_simulation` function initializes the simulation data structures and runs the simulation kernel using OpenMP offloading.