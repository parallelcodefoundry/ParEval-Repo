#include "XSbench_header.hpp"

int main(int argc, char *argv[]) {
    // =====================================================================
    // Initialization & Command Line Read-In
    // =====================================================================
    int version = 20;
    int mype = 0; // Kokkos doesn't inherently have MPI ranks.  This will need adjustment for MPI.
    double omp_start, omp_end;
    int nprocs = 1; // Kokkos doesn't inherently have MPI ranks.  This will need adjustment for MPI.
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

    // Kokkos View for SimulationData
    Kokkos::View<SimulationData*> sd_view("SD_View", Kokkos::HostSpace(),1);
    sd_view[0] = SD;


    // Run simulation using Kokkos::parallel_for
    Kokkos::Timer timer;
    if (in.simulation_method == EVENT_BASED) {
        if (in.kernel_id == 0) {
            Kokkos::parallel_for(in.lookups, [&](const int i) {
                //Kokkos-ified baseline kernel here. This requires significant restructuring.  See below.
            });
            profile.kernel_time = timer.seconds();

        } else if (in.kernel_id == 1) {
            //Kokkos::parallel_for for Optimization 1
        } else if (in.kernel_id == 2) {
            //Kokkos::parallel_for for Optimization 2
        } else if (in.kernel_id == 3) {
            //Kokkos::parallel_for for Optimization 3
        } else if (in.kernel_id == 4) {
            //Kokkos::parallel_for for Optimization 4
        } else if (in.kernel_id == 5) {
            //Kokkos::parallel_for for Optimization 5
        } else if (in.kernel_id == 6) {
            //Kokkos::parallel_for for Optimization 6
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


    //End Simulation Timer
    omp_end = omp_start + timer.seconds();


    //Reduce verification results (Kokkos::parallel_reduce)
    Kokkos::parallel_reduce(in.lookups, KOKKOS_LAMBDA (const int i, unsigned long long &lsum) {
        lsum += sd_view[0].verification[i];
    }, verification);

    //Data transfer from device (if needed)  Kokkos handles this automatically for Kokkos::View in HostSpace

    release_memory(sd_view[0]); //Note:  This needs to be adapted to correctly free Kokkos Views


    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result =
            print_results(in, mype, omp_end - omp_start, nprocs, verification);

    print_profile(profile, in);

    return is_invalid_result;
}