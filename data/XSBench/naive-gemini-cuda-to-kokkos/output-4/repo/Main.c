#include "XSbench_header.hpp"

int main(int argc, char *argv[]) {
    // =====================================================================
    // Initialization & Command Line Read-In
    // =====================================================================
    int version = 20;
    int mype = 0; // Kokkos doesn't inherently have MPI ranks; this will need adjustment for MPI.
    double omp_start, omp_end;
    int nprocs = 1; // Similar to mype, this needs adjustment for MPI in Kokkos.
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

    Kokkos::Timer timer;

    // Run simulation using Kokkos
    Kokkos::parallel_for(Kokkos::RangePolicy<>(0, in.lookups), [&](int i) {
            // ... Kokkos-based implementation of the chosen kernel ...
        });


    // Choose the appropriate kernel based on kernel_id.  This needs to be implemented with Kokkos::parallel_for.  The code below is illustrative and needs replacing.  The actual Kokkos kernels will be similar to the CUDA kernels, but use Kokkos::parallel_for and Kokkos::View.

    if (in.simulation_method == EVENT_BASED) {
        if (in.kernel_id == 0) {
            //Kokkos::parallel_for(...); //Implement baseline event-based simulation in Kokkos
            verification = run_event_based_simulation_baseline_kokkos(in, SD, mype, &profile);
        } else if (in.kernel_id == 1) {
            //Kokkos::parallel_for(...); //Implement Optimization 1 in Kokkos
            verification = run_event_based_simulation_optimization_1_kokkos(in, SD, mype);
        } else if (in.kernel_id == 2) {
            //Kokkos::parallel_for(...); //Implement Optimization 2 in Kokkos
            verification = run_event_based_simulation_optimization_2_kokkos(in, SD, mype);
        } else if (in.kernel_id == 3) {
            //Kokkos::parallel_for(...); //Implement Optimization 3 in Kokkos
            verification = run_event_based_simulation_optimization_3_kokkos(in, SD, mype);
        } else if (in.kernel_id == 4) {
            //Kokkos::parallel_for(...); //Implement Optimization 4 in Kokkos
            verification = run_event_based_simulation_optimization_4_kokkos(in, SD, mype);
        } else if (in.kernel_id == 5) {
            //Kokkos::parallel_for(...); //Implement Optimization 5 in Kokkos
            verification = run_event_based_simulation_optimization_5_kokkos(in, SD, mype);
        } else if (in.kernel_id == 6) {
            //Kokkos::parallel_for(...); //Implement Optimization 6 in Kokkos
            verification = run_event_based_simulation_optimization_6_kokkos(in, SD, mype);
        } else {
            printf("Error: No kernel ID %d found!\n", in.kernel_id);
            exit(1);
        }
    } else {
        printf(
                "History-based simulation not implemented in Kokkos code. Instead,\nuse "
                "the event-based method with \"-m event\" argument.\n");
        exit(1);
    }

    if (mype == 0) {
        printf("\n");
        printf("Simulation complete.\n");
    }


    // End Simulation Timer
    omp_end = get_time();

    // Release device memory  (Kokkos handles memory management automatically, but you need to free any manually allocated memory)
    release_memory(SD);

    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result =
            print_results(in, mype, omp_end - omp_start, nprocs, verification);

    print_profile(profile, in);

    return is_invalid_result;
}