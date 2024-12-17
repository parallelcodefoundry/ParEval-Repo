#include "XSbench_header.h"
#include <omp.h>

////////////////////////////////////////////////////////////////////////////////////
// BASELINE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////

unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile)
{
    double start = get_time();
    // Move Data to GPU
    SimulationData GSD = SD; // In OpenMP, we directly use the host data
    profile->host_to_device_time = get_time() - start;

    ////////////////////////////////////////////////////////////////////////////////
    // Configure & Launch Simulation Kernel
    ////////////////////////////////////////////////////////////////////////////////
    if (mype == 0) printf("Running baseline event-based simulation...\n");

    int nthreads = 256;
    int nblocks = (in.lookups + nthreads - 1) / nthreads;

    int nwarmups = in.num_warmups;
    start = 0.0;
    for (int i = 0; i < in.num_iterations + nwarmups; i++) {
        if (i == nwarmups) {
            #pragma omp target update from(GSD.verification[0:in.lookups])
            start = get_time();
        }
        #pragma omp target teams distribute parallel for simd collapse(2) \
            map(to: GSD.num_nucs[0:GSD.length_num_nucs], \
                GSD.concs[0:GSD.length_concs], \
                GSD.mats[0:GSD.length_mats], \
                GSD.unionized_energy_array[0:GSD.length_unionized_energy_array], \
                GSD.index_grid[0:GSD.length_index_grid], \
                GSD.nuclide_grid[0:GSD.length_nuclide_grid]) \
            map(tofrom: GSD.verification[0:in.lookups])
        for (int block = 0; block < nblocks; block++) {
            for (int thread = 0; thread < nthreads; thread++) {
                int i = block * nthreads + thread;
                if (i < in.lookups) {
                    xs_lookup_kernel_baseline(in, GSD, i);
                }
            }
        }
    }
    #pragma omp target update from(GSD.verification[0:in.lookups])
    profile->kernel_time = get_time() - start;

    ////////////////////////////////////////////////////////////////////////////////
    // Reduce Verification Results
    ////////////////////////////////////////////////////////////////////////////////

    if (mype == 0) printf("Reducing verification results...\n");
    start = get_time();

    unsigned long verification_scalar = 0;
    for (int i = 0; i < in.lookups; i++)
        verification_scalar += GSD.verification[i];

    profile->device_to_host_time = get_time() - start;

    return verification_scalar;
}

void xs_lookup_kernel_baseline(Inputs in, SimulationData GSD, int i)
{
    // The lookup ID. Used to set the seed, and to store the verification value
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
    calculate_macro_xs(
        p_energy,        // Sampled neutron energy (in lethargy)
        mat,             // Sampled material type index neutron is in
        in.n_isotopes,   // Total number of isotopes in simulation
        in.n_gridpoints, // Number of gridpoints per isotope in simulation
        GSD.num_nucs,     // 1-D array with number of nuclides per material
        GSD.concs,        // Flattened 2-D array with concentration of each nuclide in each material
        GSD.unionized_energy_array, // 1-D Unionized energy array
        GSD.index_grid,   // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
        GSD.nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
        GSD.mats,         // Flattened 2-D array with nuclide indices defining composition of each type of material
        macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
        in.grid_type,    // Lookup type (nuclide, hash, or unionized)
        in.hash_bins,    // Number of hash bins used (if using hash lookup type)
        GSD.max_num_nucs  // Maximum number of nuclides present in any material
    );

    // For verification, and to prevent the compiler from optimizing
    // all work out, we interrogate the returned macro_xs_vector array
    // to find its maximum value index, then increment the verification
    // value by that index.
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

// The rest of the device functions should be translated similarly, using OpenMP pragmas where necessary.