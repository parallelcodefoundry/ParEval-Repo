#include "XSbench_header.hpp"

int main(int argc, char *argv[]) {
    // =====================================================================
    // Initialization & Command Line Read-In
    // =====================================================================
    int version = 20;
    int mype = 0; // Kokkos doesn't inherently have MPI ranks, this will need adjustment for MPI
    double omp_start, omp_end;
    int nprocs = 1; // Kokkos doesn't inherently have MPI ranks, this will need adjustment for MPI
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

    // Run simulation using Kokkos::parallel_for
    if (in.simulation_method == EVENT_BASED) {
        Kokkos::View<unsigned long*, Kokkos::HostSpace> h_verification("h_verification", in.lookups);

        if (in.kernel_id == 0) {
            run_event_based_simulation_baseline(in, SD, mype, &profile, h_verification);
        } else if (in.kernel_id == 1) {
            run_event_based_simulation_optimization_1(in, SD, mype, h_verification);
        } else if (in.kernel_id == 2) {
            run_event_based_simulation_optimization_2(in, SD, mype, h_verification);
        } else if (in.kernel_id == 3) {
            run_event_based_simulation_optimization_3(in, SD, mype, h_verification);
        } else if (in.kernel_id == 4) {
            run_event_based_simulation_optimization_4(in, SD, mype, h_verification);
        } else if (in.kernel_id == 5) {
            run_event_based_simulation_optimization_5(in, SD, mype, h_verification);
        } else if (in.kernel_id == 6) {
            run_event_based_simulation_optimization_6(in, SD, mype, h_verification);
        } else {
            printf("Error: No kernel ID %d found!\n", in.kernel_id);
            exit(1);
        }
        Kokkos::fence();
        verification = Kokkos::Experimental::sum(h_verification);
    } else {
        printf(
                "History-based simulation not implemented in Kokkos code. Instead,\nuse "
                "the event-based method with \"-m event\" argument.\n");
        exit(1);
    }

    profile.kernel_time = timer.seconds();


    if (mype == 0) {
        printf("\n");
        printf("Simulation complete.\n");
    }

    // End Simulation Timer
    omp_end = get_time();

    // Release device memory (Kokkos handles this automatically for Views)
    release_memory(SD);

    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result =
            print_results(in, mype, omp_end - omp_start, nprocs, verification);

    print_profile(profile, in);

    return is_invalid_result;
}