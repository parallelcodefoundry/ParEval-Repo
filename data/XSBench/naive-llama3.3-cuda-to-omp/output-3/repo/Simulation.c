#include "XSbench_header.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

// BASELINE FUNCTIONS

unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile)
{
    double start = get_time();
    // Move Data to target
    SimulationData GSD = move_simulation_data_to_target(in, mype, SD);
    profile->host_to_device_time = get_time() - start;

    // Configure & Launch Simulation Kernel
    if (mype == 0) printf("Running baseline event-based simulation...\n");

    int nthreads = 256;
    int nblocks = ceil((double)in.lookups / (double)nthreads);

    int nwarmups = in.num_warmups;
    start = 0.0;
    for (int i = 0; i < in.num_iterations + nwarmups; i++) {
        if (i == nwarmups) {
            #pragma omp target exit data map(delete: GSD[0:GSD.length_verification])
            start = get_time();
        }
        #pragma omp target teams distribute map(to: in, GSD[0:GSD.length_verification]) map(from: GSD.verification[0:in.lookups])
        for (int i = 0; i < nblocks; i++) {
            #pragma omp parallel for
            for (int j = 0; j < nthreads; j++) {
                int idx = i * nthreads + j;
                if (idx >= in.lookups) continue;

                // Set the initial seed value
                uint64_t seed = STARTING_SEED;

                // Forward seed to lookup index (we need 2 samples per lookup)
                seed = fast_forward_LCG(seed, 2 * idx);

                // Randomly pick an energy and material for the particle
                double p_energy = LCG_random_double(&seed);
                int mat = pick_mat(&seed);

                double macro_xs_vector[5] = {0};

                // Perform macroscopic Cross Section Lookup
                calculate_macro_xs(p_energy, mat, in.n_isotopes, in.n_gridpoints, GSD.num_nucs, GSD.concs, GSD.unionized_energy_array, GSD.index_grid, GSD.nuclide_grid, GSD.mats, macro_xs_vector, in.grid_type, in.hash_bins, GSD.max_num_nucs);

                // For verification, and to prevent the compiler from optimizing
                // all work out, we interrogate the returned macro_xs_vector array
                // to find its maximum value index, then increment the verification
                // value by that index. In this implementation, we have each thread
                // write to its thread_id index in an array, which we will reduce
                // with a thrust reduction kernel after the main simulation kernel.
                double max = -1.0;
                int max_idx = 0;
                for (int k = 0; k < 5; k++) {
                    if (macro_xs_vector[k] > max) {
                        max = macro_xs_vector[k];
                        max_idx = k;
                    }
                }
                GSD.verification[idx] = max_idx + 1;
            }
        }
    profile->kernel_time = get_time() - start;

    // Reduce Verification Results
    if (mype == 0) printf("Reducing verification results...\n");
    start = get_time();
    unsigned long verification_scalar = 0;
    for (int i = 0; i < in.lookups; i++)
        verification_scalar += GSD.verification[i];
    profile->device_to_host_time = get_time() - start;

    release_target_memory(GSD);

    return verification_scalar;
}

// In this kernel, we perform a single lookup with each thread. Threads within a warp
// do not really have any relation to each other, and divergence due to high nuclide count fuel
// material lookups are costly. This kernel constitutes baseline performance.
void xs_lookup_kernel_baseline(Inputs in, SimulationData GSD) {
    // The lookup ID. Used to set the seed, and to store the verification value
    int i = omp_get_thread_num();

    if (i >= in.lookups)
        return;

    // Set the initial seed value
    uint64_t seed = STARTING_SEED;

    // Forward seed to lookup index (we need 2 samples per lookup)
    seed = fast_forward_LCG(seed, 2 * i);

    // Randomly pick an energy and material for the particle
    double p_energy = LCG_random_double(&seed);
    int mat = pick_mat(&seed);

    double macro_xs_vector[5] = {0};

    // Perform macroscopic Cross Section Lookup
    calculate_macro_xs(p_energy, mat, in.n_isotopes, in.n_gridpoints, GSD.num_nucs, GSD.concs, GSD.unionized_energy_array, GSD.index_grid, GSD.nuclide_grid, GSD.mats, macro_xs_vector, in.grid_type, in.hash_bins, GSD.max_num_nucs);

    // For verification, and to prevent the compiler from optimizing
    // all work out, we interrogate the returned macro_xs_vector array
    // to find its maximum value index, then increment the verification
    // value by that index. In this implementation, we have each thread
    // write to its thread_id index in an array, which we will reduce
    // with a thrust reduction kernel after the main simulation kernel.
    double max = -1.0;
    int max_idx = 0;
    for (int j = 0; j < 5; j++) {
        if (macro_xs_vector[j] > max) {
            max = macro_xs_vector[j];
            max_idx = j;
        }
    }
    GSD.verification[i] = max_idx + 1;
}

// Calculates the microscopic cross section for a given nuclide & energy
void calculate_micro_xs(double p_energy, int nuc, long n_isotopes, long n_gridpoints, double* egrid, int* index_data, NuclideGridPoint* nuclide_grids, long idx, double* xs_vector, int grid_type, int hash_bins) {
    // Variables
    double f;
    NuclideGridPoint* low, * high;

    // If using only the nuclide grid, we must perform a binary search
    // to find the energy location in this particular nuclide's grid.
    if (grid_type == NUCLIDE) {
        // Perform binary search on the Nuclide Grid to find the index
        idx = grid_search_nuclide(n_gridpoints, p_energy, &nuclide_grids[nuc * n_gridpoints], 0, n_gridpoints - 1);

        // pull ptr from nuclide grid and check to ensure that
        // we're not reading off the end of the nuclide's grid
        if (idx == n_gridpoints - 1)
            low = &nuclide_grids[nuc * n_gridpoints + idx - 1];
        else
            low = &nuclide_grids[nuc * n_gridpoints + idx];
    }
    else if (grid_type == UNIONIZED) // Unionized Energy Grid - we already know the index, no binary search needed.
    {
        // pull ptr from energy grid and check to ensure that
        // we're not reading off the end of the nuclide's grid
        if (index_data[idx * n_isotopes + nuc] == n_gridpoints - 1)
            low = &nuclide_grids[nuc * n_gridpoints + index_data[idx * n_isotopes + nuc] - 1];
        else
            low = &nuclide_grids[nuc * n_gridpoints + index_data[idx * n_isotopes + nuc]];
    }
    else // Hash grid
    {
        // load lower bounding index
        int u_low = index_data[idx * n_isotopes + nuc];

        // Determine higher bounding index
        int u_high;
        if (idx == hash_bins - 1)
            u_high = n_gridpoints - 1;
        else
            u_high = index_data[(idx + 1) * n_isotopes + nuc] + 1;

        // Check edge cases to make sure energy is actually between these
        // Then, if things look good, search for gridpoint in the nuclide grid
        // within the lower and higher limits we've calculated.
        double e_low = nuclide_grids[nuc * n_gridpoints + u_low].energy;
        double e_high = nuclide_grids[nuc * n_gridpoints + u_high].energy;
        int lower;
        if (p_energy <= e_low)
            lower = 0;
        else if (p_energy >= e_high)
            lower = n_gridpoints - 1;
        else
            lower = grid_search_nuclide(n_gridpoints, p_energy, &nuclide_grids[nuc * n_gridpoints], u_low, u_high);

        if (lower == n_gridpoints - 1)
            low = &nuclide_grids[nuc * n_gridpoints + lower - 1];
        else
            low = &nuclide_grids[nuc * n_gridpoints + lower];
    }

    high = low + 1;

    // calculate the re-useable interpolation factor
    f = (high->energy - p_energy) / (high->energy - low->energy);

    // Total XS
    xs_vector[0] = high->total_xs - f * (high->total_xs - low->total_xs);

    // Elastic XS
    xs_vector[1] = high->elastic_xs - f * (high->elastic_xs - low->elastic_xs);

    // Absorbtion XS
    xs_vector[2] = high->absorbtion_xs - f * (high->absorbtion_xs - low->absorbtion_xs);

    // Fission XS
    xs_vector[3] = high->fission_xs - f * (high->fission_xs - low->fission_xs);

    // Nu Fission XS
    xs_vector[4] = high->nu_fission_xs - f * (high->nu_fission_xs - low->nu_fission_xs);
}

// Calculates macroscopic cross section based on a given material & energy
void calculate_macro_xs(double p_energy, int mat, long n_isotopes, long n_gridpoints, int* num_nucs, double* concs, double* egrid, int* index_data, NuclideGridPoint* nuclide_grids, int* mats, double* macro_xs_vector, int grid_type, int hash_bins, int max_num_nucs) {
    int p_nuc; // the nuclide we are looking up
    long idx = -1;
    double conc; // the concentration of the nuclide in the material

    // cleans out macro_xs_vector
    for (int k = 0; k < 5; k++)
        macro_xs_vector[k] = 0;

    // If we are using the unionized energy grid (UEG), we only
    // need to perform 1 binary search per macroscopic lookup.
    // If we are using the nuclide grid search, it will have to be
    // done inside of the "calculate_micro_xs" function for each different
    // nuclide in the material.
    if (grid_type == UNIONIZED)
        idx = grid_search(n_isotopes * n_gridpoints, p_energy, egrid);
    else if (grid_type == HASH) {
        double du = 1.0 / hash_bins;
        idx = p_energy / du;
    }

    // Once we find the pointer array on the UEG, we can pull the data
    // from the respective nuclide grids, as well as the nuclide
    // concentration data for the material
    // Each nuclide from the material needs to have its micro-XS array
    // looked up & interpolatied (via calculate_micro_xs). Then, the
    // micro XS is multiplied by the concentration of that nuclide
    // in the material, and added to the total macro XS array.
    // (Independent -- though if parallelizing, must use atomic operations
    //  or otherwise control access to the xs_vector and macro_xs_vector to
    //  avoid simulataneous writing to the same data structure)
    for (int j = 0; j < num_nucs[mat]; j++) {
        double xs_vector[5];
        p_nuc = mats[mat * max_num_nucs + j];
        conc = concs[mat * max_num_nucs + j];
        calculate_micro_xs(p_energy, p_nuc, n_isotopes, n_gridpoints, egrid, index_data, nuclide_grids, idx, xs_vector, grid_type, hash_bins);
        for (int k = 0; k < 5; k++)
            macro_xs_vector[k] += xs_vector[k] * conc;
    }
}

// binary search for energy on unionized energy grid
// returns lower index
long grid_search(long n, double quarry, double* A) {
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

// binary search for energy on nuclide energy grid
long grid_search_nuclide(long n, double quarry, NuclideGridPoint* A, long low, long high) {
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

// picks a material based on a probabilistic distribution
int pick_mat(uint64_t* seed) {
    // I have a nice spreadsheet supporting these numbers. They are
    // the fractions (by volume) of material in the core. Not a
    // *perfect* approximation of where XS lookups are going to occur,
    // but this will do a good job of biasing the system nonetheless.

    // Also could be argued that doing fractions by weight would be
    // a better approximation, but volume does a good enough job for now.

    double dist[12];
    dist[0] = 0.140; // fuel
    dist[1] = 0.052; // cladding
    dist[2] = 0.275; // cold, borated water
    dist[3] = 0.134; // hot, borated water
    dist[4] = 0.154; // RPV
    dist[5] = 0.064; // Lower, radial reflector
    dist[6] = 0.066; // Upper reflector / top plate
    dist[7] = 0.055; // bottom plate
    dist[8] = 0.008; // bottom nozzle
    dist[9] = 0.015; // top nozzle
    dist[10] = 0.025; // top of fuel assemblies
    dist[11] = 0.013; // bottom of fuel assemblies

    double roll = LCG_random_double(seed);

    // makes a pick based on the distro
    for (int i = 0; i < 12; i++) {
        double running = 0;
        for (int j = i; j > 0; j--)
            running += dist[j];
        if (roll < running)
            return i;
    }

    return 0;
}

double LCG_random_double(uint64_t* seed) {
    // LCG parameters
    const uint64_t m = 9223372036854775808ULL; // 2^63
    const uint64_t a = 2806196910506780709ULL;
    const uint64_t c = 1ULL;
    *seed = (a * (*seed) + c) % m;
    return (double)*seed / (double)m;
}

uint64_t fast_forward_LCG(uint64_t seed, uint64_t n) {
    // LCG parameters
    const uint64_t m = 9223372036854775808ULL; // 2^63
    uint64_t a = 2806196910506780709ULL;
    uint64_t c = 1ULL;

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

// OPTIMIZED VARIANT FUNCTIONS

// ... (rest of the code remains the same)