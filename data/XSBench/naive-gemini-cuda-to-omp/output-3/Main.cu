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
        if (in.kernel_id == 0) {
            #pragma omp target teams distribute parallel for map(to:in,SD) map(from:verification)
            for (int i = 0; i < in.lookups; i++) {
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
                    p_energy, // Sampled neutron energy (in lethargy)
                    mat, // Sampled material type index neutron is in
                    in.n_isotopes, // Total number of isotopes in simulation
                    in.n_gridpoints, // Number of gridpoints per isotope in simulation
                    SD.num_nucs, // 1-D array with number of nuclides per material
                    SD.concs, // Flattened 2-D array with concentration of each nuclide in each material
                    SD.unionized_energy_array, // 1-D Unionized energy array
                    SD.index_grid, // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
                    SD.nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
                    SD.mats, // Flattened 2-D array with nuclide indices defining composition of each type of material
                    macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
                    in.grid_type, // Lookup type (nuclide, hash, or unionized)
                    in.hash_bins, // Number of hash bins used (if using hash lookup type)
                    SD.max_num_nucs // Maximum number of nuclides present in any material
                );

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
                SD.verification[i] = max_idx + 1;
            }
        } else if (in.kernel_id == 1) {
            #pragma omp target teams distribute parallel for map(to:in,SD) map(from:verification)
            for (int i = 0; i < in.lookups; i++) {
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
                    p_energy, // Sampled neutron energy (in lethargy)
                    mat, // Sampled material type index neutron is in
                    in.n_isotopes, // Total number of isotopes in simulation
                    in.n_gridpoints, // Number of gridpoints per isotope in simulation
                    SD.num_nucs, // 1-D array with number of nuclides per material
                    SD.concs, // Flattened 2-D array with concentration of each nuclide in each material
                    SD.unionized_energy_array, // 1-D Unionized energy array
                    SD.index_grid, // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
                    SD.nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
                    SD.mats, // Flattened 2-D array with nuclide indices defining composition of each type of material
                    macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
                    in.grid_type, // Lookup type (nuclide, hash, or unionized)
                    in.hash_bins, // Number of hash bins used (if using hash lookup type)
                    SD.max_num_nucs // Maximum number of nuclides present in any material
                );

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
                SD.verification[i] = max_idx + 1;
            }
        } else if (in.kernel_id == 2) {
            #pragma omp target teams distribute parallel for map(to:in,SD) map(from:verification)
            for (int i = 0; i < in.lookups; i++) {
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
                    p_energy, // Sampled neutron energy (in lethargy)
                    mat, // Sampled material type index neutron is in
                    in.n_isotopes, // Total number of isotopes in simulation
                    in.n_gridpoints, // Number of gridpoints per isotope in simulation
                    SD.num_nucs, // 1-D array with number of nuclides per material
                    SD.concs, // Flattened 2-D array with concentration of each nuclide in each material
                    SD.unionized_energy_array, // 1-D Unionized energy array
                    SD.index_grid, // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
                    SD.nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
                    SD.mats, // Flattened 2-D array with nuclide indices defining composition of each type of material
                    macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
                    in.grid_type, // Lookup type (nuclide, hash, or unionized)
                    in.hash_bins, // Number of hash bins used (if using hash lookup type)
                    SD.max_num_nucs // Maximum number of nuclides present in any material
                );

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
                SD.verification[i] = max_idx + 1;
            }
        } else if (in.kernel_id == 3) {
            #pragma omp target teams distribute parallel for map(to:in,SD) map(from:verification)
            for (int i = 0; i < in.lookups; i++) {
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
                    p_energy, // Sampled neutron energy (in lethargy)
                    mat, // Sampled material type index neutron is in
                    in.n_isotopes, // Total number of isotopes in simulation
                    in.n_gridpoints, // Number of gridpoints per isotope in simulation
                    SD.num_nucs, // 1-D array with number of nuclides per material
                    SD.concs, // Flattened 2-D array with concentration of each nuclide in each material
                    SD.unionized_energy_array, // 1-D Unionized energy array
                    SD.index_grid, // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
                    SD.nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
                    SD.mats, // Flattened 2-D array with nuclide indices defining composition of each type of material
                    macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
                    in.grid_type, // Lookup type (nuclide, hash, or unionized)
                    in.hash_bins, // Number of hash bins used (if using hash lookup type)
                    SD.max_num_nucs // Maximum number of nuclides present in any material
                );

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
                SD.verification[i] = max_idx + 1;
            }
        } else if (in.kernel_id == 4) {
            #pragma omp target teams distribute parallel for map(to:in,SD) map(from:verification)
            for (int i = 0; i < in.lookups; i++) {
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
                    p_energy, // Sampled neutron energy (in lethargy)
                    mat, // Sampled material type index neutron is in
                    in.n_isotopes, // Total number of isotopes in simulation
                    in.n_gridpoints, // Number of gridpoints per isotope in simulation
                    SD.num_nucs, // 1-D array with number of nuclides per material
                    SD.concs, // Flattened 2-D array with concentration of each nuclide in each material
                    SD.unionized_energy_array, // 1-D Unionized energy array
                    SD.index_grid, // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
                    SD.nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
                    SD.mats, // Flattened 2-D array with nuclide indices defining composition of each type of material
                    macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
                    in.grid_type, // Lookup type (nuclide, hash, or unionized)
                    in.hash_bins, // Number of hash bins used (if using hash lookup type)
                    SD.max_num_nucs // Maximum number of nuclides present in any material
                );

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
                SD.verification[i] = max_idx + 1;
            }
        } else if (in.kernel_id == 5) {
            #pragma omp target teams distribute parallel for map(to:in,SD) map(from:verification)
            for (int i = 0; i < in.lookups; i++) {
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
                    p_energy, // Sampled neutron energy (in lethargy)
                    mat, // Sampled material type index neutron is in
                    in.n_isotopes, // Total number of isotopes in simulation
                    in.n_gridpoints, // Number of gridpoints per isotope in simulation
                    SD.num_nucs, // 1-D array with number of nuclides per material
                    SD.concs, // Flattened 2-D array with concentration of each nuclide in each material
                    SD.unionized_energy_array, // 1-D Unionized energy array
                    SD.index_grid, // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
                    SD.nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
                    SD.mats, // Flattened 2-D array with nuclide indices defining composition of each type of material
                    macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
                    in.grid_type, // Lookup type (nuclide, hash, or unionized)
                    in.hash_bins, // Number of hash bins used (if using hash lookup type)
                    SD.max_num_nucs // Maximum number of nuclides present in any material
                );

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
                SD.verification[i] = max_idx + 1;
            }
        } else if (in.kernel_id == 6) {
            #pragma omp target teams distribute parallel for map(to:in,SD) map(from:verification)
            for (int i = 0; i < in.lookups; i++) {
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
                    p_energy, // Sampled neutron energy (in lethargy)
                    mat, // Sampled material type index neutron is in
                    in.n_isotopes, // Total number of isotopes in simulation
                    in.n_gridpoints, // Number of gridpoints per isotope in simulation
                    SD.num_nucs, // 1-D array with number of nuclides per material
                    SD.concs, // Flattened 2-D array with concentration of each nuclide in each material
                    SD.unionized_energy_array, // 1-D Unionized energy array
                    SD.index_grid, // Flattened 2-D grid holding indices into nuclide grid for each unionized energy level
                    SD.nuclide_grid, // Flattened 2-D grid holding energy levels and XS_data for all nuclides in simulation
                    SD.mats, // Flattened 2-D array with nuclide indices defining composition of each type of material
                    macro_xs_vector, // 1-D array with result of the macroscopic cross section (5 different reaction channels)
                    in.grid_type, // Lookup type (nuclide, hash, or unionized)
                    in.hash_bins, // Number of hash bins used (if using hash lookup type)
                    SD.max_num_nucs // Maximum number of nuclides present in any material
                );

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
                SD.verification[i] = max_idx + 1;
            }
        } else {
            printf("Error: No kernel ID %d found!\n", in.kernel_id);
            exit(1);
        }
    } else {
        printf(
            "History-based simulation not implemented in CUDA code. Instead,\nuse "
            "the event-based method with \"-m event\" argument.\n");
        exit(1);
    }

    if (mype == 0) {
        printf("\n");
        printf("Simulation complete.\n");
    }

    // End Simulation Timer
    omp_end = get_time();

    // Release device memory
    release_memory(SD);

    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result =
        print_results(in, mype, omp_end - omp_start, nprocs, verification);

    print_profile(profile, in);

    return is_invalid_result;
}