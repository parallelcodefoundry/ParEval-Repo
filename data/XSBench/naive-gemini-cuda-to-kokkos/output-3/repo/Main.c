#include "XSbench_header.hpp"

int main(int argc, char *argv[]) {
    // =====================================================================
    // Initialization & Command Line Read-In
    // =====================================================================
    int version = 20;
    int mype = 0; // Kokkos doesn't inherently have MPI ranks; this needs adjustment for parallel runs.
    double omp_start, omp_end;
    int nprocs = 1; // Similarly, needs adjustment for parallel Kokkos runs.
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

    // Run simulation using Kokkos Views
    Kokkos::View<SimulationData*, Kokkos::HostSpace> SD_view(SD);
    Kokkos::View<SimulationData*, Kokkos::CudaSpace> SD_cuda_view = Kokkos::create_mirror_view(Kokkos::Cuda(), SD_view);
    Kokkos::deep_copy(SD_cuda_view, SD_view); // Copy data to the device


    if (in.simulation_method == EVENT_BASED) {
        Kokkos::Timer timer;
        if (in.kernel_id == 0) {
            verification = run_event_based_simulation_baseline(in, SD_cuda_view, mype, &profile);
            profile.kernel_time = timer.seconds();
        } else if (in.kernel_id == 1)
            verification = run_event_based_simulation_optimization_1(in, SD_cuda_view, mype);
        else if (in.kernel_id == 2)
            verification = run_event_based_simulation_optimization_2(in, SD_cuda_view, mype);
        else if (in.kernel_id == 3)
            verification = run_event_based_simulation_optimization_3(in, SD_cuda_view, mype);
        else if (in.kernel_id == 4)
            verification = run_event_based_simulation_optimization_4(in, SD_cuda_view, mype);
        else if (in.kernel_id == 5)
            verification = run_event_based_simulation_optimization_5(in, SD_cuda_view, mype);
        else if (in.kernel_id == 6)
            verification = run_event_based_simulation_optimization_6(in, SD_cuda_view, mype);
        else {
            printf("Error: No kernel ID %d found!\n", in.kernel_id);
            exit(1);
        }
    } else {
        printf(
                "History-based simulation not implemented in Kokkos code. Instead,\nuse "
                "the event-based method with \"-m event\" argument.\n");
        exit(1);
    }

    Kokkos::deep_copy(SD_view, SD_cuda_view); //Copy back from device to host

    if (mype == 0) {
        printf("\n");
        printf("Simulation complete.\n");
    }


    // End Simulation Timer
    omp_end = get_time();

    // Release device memory (implicitly handled by Kokkos::View destructor)
    release_memory(SD);

    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result =
            print_results(in, mype, omp_end - omp_start, nprocs, verification);

    print_profile(profile, in);

    return is_invalid_result;
}