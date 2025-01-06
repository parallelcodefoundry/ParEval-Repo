#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>

int main(int argc, char *argv[])
{
    Kokkos::initialize(argc, argv);
    {
        int version = 4;

        srand(time(NULL));

        Input I = set_default_input();
        read_CLI(argc, argv, &I);

        // Calculate Number of 3D Source Regions
        I.source_3D_regions = (int)ceil((double)I.source_2D_regions *
                                         I.coarse_axial_intervals / I.decomp_assemblies_ax);

        logo(version);
        print_input_summary(I);

        center_print("INITIALIZATION", 79);
        border_print();

        // Build Source Data
        Source_Arrays SA_h, SA_d;
        Source *sources_h = initialize_sources(I, &SA_h);
        Source *sources_d = initialize_device_sources(I, &SA_h, &SA_d, sources_h);

        // Build Exponential Table
        Table *table_d = nullptr;
#ifdef TABLE
        printf("Building Exponential Table...\n");
        Table table = buildExponentialTable();
        table_d = new Table(table); // Allocate on host
#endif

        // Setup Kokkos execution space
        Kokkos::View<curandState *, Kokkos::DefaultExecutionSpace> RNG_states("RNG_states", I.streams);
        setup_kernel(RNG_states, I);

        // Allocate Some Flux State vectors to randomly pick from
        printf("Setting up Flux State Vectors...\n");
        float *flux_states;
        int N_flux_states = 10000;
        assert(I.segments >= N_flux_states);
        Kokkos::View<float *, Kokkos::DefaultExecutionSpace> flux_states_view("flux_states", N_flux_states * I.egroups);
        init_flux_states(flux_states_view, N_flux_states, I, RNG_states);

        printf("Initialization Complete.\n");
        border_print();
        center_print("SIMULATION", 79);
        border_print();

        printf("Attenuating fluxes across segments...\n");

        // Run Simulation Kernel Loop
        run_kernel(I, sources_d, SA_d, table_d, RNG_states, flux_states_view, N_flux_states);

        float *host_flux_states = (float *)malloc(N_flux_states * I.egroups * sizeof(float));
        Kokkos::deep_copy(host_flux_states, flux_states_view);

        printf("Simulation Complete.\n");

        border_print();
        center_print("RESULTS SUMMARY", 79);
        border_print();

        double tpi = ((double)(time / 1000.0) /
                      (double)I.segments / (double)I.egroups) * 1.0e9;
        printf("%-25s%.3f seconds\n", "Runtime:", time / 1000.0);
        printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
        border_print();

        free(host_flux_states);
#ifdef TABLE
        delete table_d; // Clean up table if allocated
#endif
    }
    Kokkos::finalize();
    return 0;
}