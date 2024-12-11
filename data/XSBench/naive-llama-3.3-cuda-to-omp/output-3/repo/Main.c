#include "XSbench_header.cuh"

int main(int argc, char *argv[]) {
    // =====================================================================
    // Initialization & Command Line Read-In
    // =====================================================================
    int version = 20;
    double omp_start, omp_end;
    unsigned long long verification;

    // Process CLI Fields -- store in "Inputs" structure
    Inputs in = read_CLI(argc, argv);

    // Print-out of Input Summary
    print_inputs(in, 1, version);

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
        SD = grid_init_do_not_profile(in, 0);

    // If writing from file mode is selected, write all simulation data
    // structures to file
    if (in.binary_mode == WRITE)
        binary_write(in, SD);

    Profile profile;

    // =====================================================================
    // Cross Section (XS) Parallel Lookup Simulation
    // This is the section that should be profiled, as it reflects a
    // realistic continuous energy Monte Carlo macroscopic cross section
    // lookup kernel.
    // =====================================================================
    printf("\n");
    border_print();
    center_print("SIMULATION", 79);
    border_print();

    // Start Simulation Timer
    omp_start = get_time();

    // Run simulation
    if (in.simulation_method == EVENT_BASED) {
        if (in.kernel_id == 0)
            verification = run_event_based_simulation_baseline(in, SD, 0, &profile);
        else if (in.kernel_id == 1)
            verification = run_event_based_simulation_optimization_1(in, SD, 0);
        else if (in.kernel_id == 2)
            verification = run_event_based_simulation_optimization_2(in, SD, 0);
        else if (in.kernel_id == 3)
            verification = run_event_based_simulation_optimization_3(in, SD, 0);
        else if (in.kernel_id == 4)
            verification = run_event_based_simulation_optimization_4(in, SD, 0);
        else if (in.kernel_id == 5)
            verification = run_event_based_simulation_optimization_5(in, SD, 0);
        else if (in.kernel_id == 6)
            verification = run_event_based_simulation_optimization_6(in, SD, 0);
        else {
            printf("Error: No kernel ID %d found!\n", in.kernel_id);
            exit(1);
        }
    } else {
        printf(
                "History-based simulation not implemented in OpenMP-offload code. Instead,\nuse "
                "the event-based method with \"-m event\" argument.\n");
        exit(1);
    }

    printf("\n");
    printf("Simulation complete.\n");

    // End Simulation Timer
    omp_end = get_time();

    // Release device memory
    release_memory(SD);

    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result =
            print_results(in, 0, omp_end - omp_start, 1, verification);

    print_profile(profile, in);

    return is_invalid_result;
}