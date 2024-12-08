#include "SimpleMOC-kernel_header.h"
#include <omp.h>

int main(int argc, char *argv[]) {
    int version = 4;

    srand(time(NULL));

    Input I = set_default_input();
    read_CLI(argc, argv, &I);

    // Calculate Number of 3D Source Regions
    I.source_3D_regions = (int) ceil((double) I.source_2D_regions *
                                     I.coarse_axial_intervals / I.decomp_assemblies_ax);

    logo(version);

    print_input_summary(I);

    center_print("INITIALIZATION", 79);
    border_print();

    // Build Source Data
    printf("Building Source Data Arrays...\n");
    Source_Arrays SA_h, SA_d;
    Source *sources_h = initialize_sources(I, &SA_h);

    // Allocate device memory and copy data
    #pragma omp target data map(to: SA_h, sources_h[0:I.source_3D_regions]) map(from: SA_d)
    {
        Source *sources_d = initialize_device_sources(I, &SA_h, &SA_d, sources_h);

        // Build Exponential Table
        Table *table_d = NULL;
        #ifdef TABLE
        printf("Building Exponential Table...\n");
        Table table = buildExponentialTable();
        #pragma omp target enter data map(to: table)
        table_d = &table;
        #endif

        // Setup OpenMP blocks / threads
        int n_blocks = sqrt(I.segments);
        dim3 blocks(n_blocks, n_blocks);
        if (blocks.x * blocks.y < I.segments)
            blocks.x++;
        if (blocks.x * blocks.y < I.segments)
            blocks.y++;
        assert(blocks.x * blocks.y >= I.segments);

        // Setup OpenMP RNG on Device
        printf("Setting up RNG...\n");
        curandState *RNG_states;
        #pragma omp target enter data map(alloc: RNG_states[0:I.streams])
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < I.streams; ++i) {
            setup_kernel(RNG_states, I);
        }

        // Allocate Some Flux State vectors to randomly pick from
        printf("Setting up Flux State Vectors...\n");
        float *flux_states;
        int N_flux_states = 10000;
        assert(I.segments >= N_flux_states);
        #pragma omp target enter data map(alloc: flux_states[0:N_flux_states * I.egroups])
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N_flux_states; ++i) {
            init_flux_states(flux_states, N_flux_states, I, RNG_states);
        }

        printf("Initialization Complete.\n");
        border_print();
        center_print("SIMULATION", 79);
        border_print();
        printf("Attenuating fluxes across segments...\n");

        // OpenMP timer variables
        double start_time, end_time;
        start_time = omp_get_wtime();

        // Setup kernel call block parameters
        assert(I.segments % I.seg_per_thread == 0);
        n_blocks = sqrt(I.segments / I.seg_per_thread);
        dim3 blocks_k(n_blocks, n_blocks);
        if (blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread)
            blocks_k.x++;
        if (blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread)
            blocks_k.y++;
        assert(blocks_k.x * blocks_k.y >= I.segments / I.seg_per_thread);

        // Run Simulation Kernel Loop
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < blocks_k.x * blocks_k.y; ++i) {
            run_kernel(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states);
        }

        end_time = omp_get_wtime();
        double time = end_time - start_time;

        float *host_flux_states = (float *) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target update from(flux_states[0:N_flux_states * I.egroups])

        printf("Simulation Complete.\n");

        border_print();
        center_print("RESULTS SUMMARY", 79);
        border_print();

        double tpi = (time / (double) I.segments / (double) I.egroups) * 1.0e9;
        printf("%-25s%.3f seconds\n", "Runtime:", time);
        printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
        border_print();
    }

    return 0;
}