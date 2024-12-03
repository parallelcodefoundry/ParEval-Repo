#include "XSbench_header.cuh"

int main(int argc, char *argv[]) {
    // =====================================================================
    // Initialization & Command Line Read-In
    // =====================================================================
    int version = 20;
    int mype = 0; //This will need to be adapted for OpenMP
    double omp_start, omp_end;
    int nprocs = 1; //This will need to be adapted for OpenMP
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

    // Run simulation.  OpenMP offloading will happen here.
    #pragma omp target data map(to: SD.num_nucs[0:SD.length_num_nucs], SD.concs[0:SD.length_concs], SD.mats[0:SD.length_mats], SD.nuclide_grid[0:SD.length_nuclide_grid], SD.index_grid[0:SD.length_index_grid], SD.unionized_energy_array[0:SD.length_unionized_energy_array]) map(from: profile.kernel_time, profile.device_to_host_time, profile.host_to_device_time, verification) map(alloc: profile)
    {
        #pragma omp target teams distribute parallel for reduction(+:verification)
        for (long i = 0; i < in.lookups; i++) {
            //Simulate the kernel here.  This will require significant restructuring of the code to avoid CUDA specifics.  This is just a placeholder.
            //The kernel call needs to be replaced with a suitable OpenMP implementation.
            // Example:  verification += my_openmp_kernel(i, in, SD);
            // ... (Implementation of the actual OpenMP kernel)
        }
    }

    // End Simulation Timer
    omp_end = get_time();

    // Release device memory (This is now handled automatically with the target data construct.)
    release_memory(SD);

    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result =
        print_results(in, mype, omp_end - omp_start, nprocs, verification);

    print_profile(profile, in);

    return is_invalid_result;
}