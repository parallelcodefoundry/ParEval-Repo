#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>

int main(int argc, char* argv[]) {
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
        printf("Building Source Data Arrays...\n");
        Source_Arrays SA_h, SA_d;
        Source* sources_h = initialize_sources(I, &SA_h);
        Source* sources_d = initialize_device_sources(I, &SA_h, &SA_d, sources_h);
        Kokkos::fence();

        // Build Exponential Table
        Table* table_d = nullptr;
#ifdef TABLE
        printf("Building Exponential Table...\n");
        Table table = buildExponentialTable();
        Kokkos::View<Table*> table_view("table_view");
        table_view() = &table;
        table_d = table_view.data();
#endif

        // Setup CUDA blocks / threads
        int n_blocks = sqrt(I.segments);
        Kokkos::View<int*> blocks("blocks");
        blocks() = n_blocks;
        if (blocks() * blocks() < I.segments)
            blocks()++;
        if (blocks() * blocks() < I.segments)
            blocks()++;
        assert(blocks() * blocks() >= I.segments);

        // Setup CUDA RNG on Device
        printf("Setting up CUDA RNG...\n");
        Kokkos::View<curandState*> RNG_states("RNG_states", I.streams);
        Kokkos::parallel_for(Kokkos::RangePolicy<>(0, I.streams), KOKKOS_LAMBDA(const int i) {
            curand_init(1234, i, 0, &RNG_states(i));
        });
        Kokkos::fence();

        // Allocate Some Flux State vectors to randomly pick from
        printf("Setting up Flux State Vectors...\n");
        Kokkos::View<float*> flux_states("flux_states", 10000 * I.egroups);
        int N_flux_states = 10000;
        assert(I.segments >= N_flux_states);
        Kokkos::parallel_for(Kokkos::RangePolicy<>(0, N_flux_states * I.egroups), KOKKOS_LAMBDA(const int i) {
            flux_states(i) = curand_uniform(&RNG_states(i % I.streams));
        });
        Kokkos::fence();

        printf("Initialization Complete.\n");
        border_print();
        center_print("SIMULATION", 79);
        border_print();
        Kokkos::fence();
        printf("Attentuating fluxes across segments...\n");

        // CUDA timer variables
        Kokkos::Timer timer;

        // Run Simulation Kernel Loop
        timer.reset();
        Kokkos::parallel_for(Kokkos::RangePolicy<>(0, I.segments / I.seg_per_thread), KOKKOS_LAMBDA(const int i) {
            run_kernel(I, sources_d, SA_d, table_d, RNG_states.data(), flux_states.data(), N_flux_states);
        });
        Kokkos::fence();
        double time = timer.seconds();

        float* host_flux_states = (float*)malloc(N_flux_states * I.egroups * sizeof(float));
        Kokkos::deep_copy(Kokkos::View<float*>(host_flux_states, N_flux_states * I.egroups), flux_states);

        printf("Simulation Complete.\n");

        border_print();
        center_print("RESULTS SUMMARY", 79);
        border_print();

        double tpi = ((double)(time) / (double)I.segments / (double)I.egroups) * 1.0e9;
        printf("%-25s%.3f seconds\n", "Runtime:", time);
        printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
        border_print();

    }
    Kokkos::finalize();
    return 0;
}