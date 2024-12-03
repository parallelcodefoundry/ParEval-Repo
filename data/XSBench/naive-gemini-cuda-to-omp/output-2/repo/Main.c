#include "XSbench_header.cuh"

int main(int argc, char *argv[]) {
    // =====================================================================
    // Initialization & Command Line Read-In
    // =====================================================================
    int version = 20;
    int mype = 0;
    double omp_start, omp_end;
    int nprocs = 1;
    unsigned long long verification;

    // Process CLI Fields -- store in "Inputs" structure
    Inputs in = read_CLI(argc, argv);

    // Print-out of Input Summary
    if (mype == 0)
        print_inputs(in, nprocs, version);

    // =====================================================================
    // Prepare Nuclide Energy Grids, Unionized Energy Grid, & Material Data
    // This is not reflective of a real Monte Carlo simulation workload,
    // therefore, do not profile this region!
    // =====================================================================

    SimulationData SD;

    // If read from file mode is selected, skip initialization and load
    // all simulation data structures from file instead
    if (in.binary_mode == READ)
        SD = binary_read(in);
    else
        SD = grid_init_do_not_profile(in, mype);

    // If writing from file mode is selected, write all simulation data
    // structures to file
    if (in.binary_mode == WRITE && mype == 0)
        binary_write(in, SD);

    Profile profile;

    // =====================================================================
    // Cross Section (XS) Parallel Lookup Simulation
    // This is the section that should be profiled, as it reflects a
    // realistic continuous energy Monte Carlo macroscopic cross section
    // lookup kernel.
    // =====================================================================
    if (mype == 0) {
        printf("\n");
        border_print();
        center_print("SIMULATION", 79);
        border_print();
    }

    // Start Simulation Timer
    omp_start = get_time();

    // Run simulation
    if (in.simulation_method == EVENT_BASED) {
        #pragma omp target data map(to: in, SD.num_nucs[0:SD.length_num_nucs], SD.concs[0:SD.length_concs], SD.mats[0:SD.length_mats], \
                                      SD.unionized_energy_array[0:SD.length_unionized_energy_array], SD.index_grid[0:SD.length_index_grid], \
                                      SD.nuclide_grid[0:SD.length_nuclide_grid], SD.max_num_nucs) \
                        map(alloc: profile) map(from: verification)
        {
            #pragma omp target teams distribute parallel for reduction(+:verification)
            for (long i = 0; i < in.lookups; i++) {
                uint64_t seed = STARTING_SEED;
                seed = fast_forward_LCG(seed, 2 * i);
                double p_energy = LCG_random_double(&seed);
                int mat = pick_mat(&seed);
                double macro_xs_vector[5] = {0};
                calculate_macro_xs(p_energy, mat, in.n_isotopes, in.n_gridpoints, SD.num_nucs, SD.concs,
                                   SD.unionized_energy_array, SD.index_grid, SD.nuclide_grid, SD.mats,
                                   macro_xs_vector, in.grid_type, in.hash_bins, SD.max_num_nucs);

                double max = -1.0;
                int max_idx = 0;
                for (int j = 0; j < 5; j++) {
                    if (macro_xs_vector[j] > max) {
                        max = macro_xs_vector[j];
                        max_idx = j;
                    }
                }
                verification += max_idx + 1;
            }
        }
    } else {
        printf(
                "History-based simulation not implemented in OpenMP offload code. Instead,\nuse "
                "the event-based method with \"-m event\" argument.\n");
        exit(1);
    }

    if (mype == 0) {
        printf("\n");
        printf("Simulation complete.\n");
    }


    // End Simulation Timer
    omp_end = get_time();

    // Release device memory.  Not needed for OpenMP offload
    release_memory(SD);

    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result =
            print_results(in, mype, omp_end - omp_start, nprocs, verification);

    print_profile(profile, in);

    return is_invalid_result;
}