#include "XSbench_header.hpp"

int main(int argc, char *argv[]) {
    // =====================================================================
    // Initialization & Command Line Read-In
    // =====================================================================
    int version = 20;
    //int mype = 0; // Removed as Kokkos handles parallelism differently
    double omp_start, omp_end;
    //int nprocs = 1; // Removed as Kokkos handles parallelism differently
    unsigned long long verification;

    // Process CLI Fields -- store in "Inputs" structure
    Inputs in = read_CLI(argc, argv);

    // Print-out of Input Summary
    Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int& i) {
        if (i == 0) print_inputs(in, 1, version); // Assuming 1 process for now
    });


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
        SD = grid_init_do_not_profile(in, 0); // Assuming 0 for single process


    // If writing from file mode is selected, write all simulation data
    // structures to file
    if (in.binary_mode == WRITE) {
        Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int& i) {
            if (i == 0) binary_write(in, SD);
        });
    }

    Profile profile;

    // =====================================================================
    // Cross Section (XS) Parallel Lookup Simulation
    // This is the section that should be profiled, as it reflects a
    // realistic continuous energy Monte Carlo macroscopic cross section
    // lookup kernel.
    // =====================================================================
    Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int& i) {
        if (i == 0) {
            printf("\n");
            border_print();
            center_print("SIMULATION", 79);
            border_print();
        }
    });

    // Start Simulation Timer
    omp_start = get_time();

    // Run simulation using Kokkos::parallel_reduce
    Kokkos::parallel_reduce(Kokkos::RangePolicy<>(0, in.lookups),
                            KOKKOS_LAMBDA(const int& i, unsigned long long& update, const long& i_){
                                if (in.simulation_method == EVENT_BASED) {
                                    if (in.kernel_id == 0) {
                                        //Call baseline kernel here.  Requires restructuring.
                                    } else if (in.kernel_id == 1) {
                                        //Call optimization 1 kernel here. Requires restructuring.
                                    } else if (in.kernel_id == 2) {
                                        //Call optimization 2 kernel here. Requires restructuring.
                                    } else if (in.kernel_id == 3) {
                                        //Call optimization 3 kernel here. Requires restructuring.
                                    } else if (in.kernel_id == 4) {
                                        //Call optimization 4 kernel here. Requires restructuring.
                                    } else if (in.kernel_id == 5) {
                                        //Call optimization 5 kernel here. Requires restructuring.
                                    } else if (in.kernel_id == 6) {
                                        //Call optimization 6 kernel here. Requires restructuring.
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

                            },verification);



    // End Simulation Timer
    omp_end = get_time();

    // Release device memory (Kokkos handles memory management)
    release_memory(SD);

    // Final Hash Step
    verification = verification % 999983;

    // Print / Save Results and Exit
    int is_invalid_result = 1;
    Kokkos::parallel_for(1, KOKKOS_LAMBDA(const int& i) {
        if (i == 0) is_invalid_result = print_results(in, 0, omp_end - omp_start, 1, verification); // Assuming 1 process
    });

    print_profile(profile, in);

    return is_invalid_result;
}