#include "SimpleMOC-kernel_header.h"
#include <omp.h>

int main(int argc, char *argv[]) {
    int version = 4;

    srand(time(NULL));

    Input I = set_default_input();
    read_CLI(argc, argv, &I);

    // Calculate Number of 3D Source Regions
    I.source_3D_regions = (int) ceil((double)I.source_2D_regions *
                                     I.coarse_axial_intervals / I.decomp_assemblies_ax);

    logo(version);

    print_input_summary(I);

    center_print("INITIALIZATION", 79);
    border_print();

    // Build Source Data
    printf("Building Source Data Arrays...\n");
    Source_Arrays SA_h, SA_d;
    Source *sources_h = initialize_sources(I, &SA_h);
    Source *sources_d = initialize_device_sources(I, &SA_h, &SA_d, sources_h);

    // Build Exponential Table
    Table *table_d = NULL;
#ifdef TABLE
    printf("Building Exponential Table...\n");
    Table table = buildExponentialTable();
    table_d = &table;
#endif

    // Setup RNG states on device
    printf("Setting up RNG states...\n");
    curandState *RNG_states = (curandState *)malloc(I.streams * sizeof(curandState));
    #pragma omp target data map(to: RNG_states[0:I.streams])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < I.streams; i++) {
            setup_kernel(&RNG_states[i], I);
        }
    }

    // Allocate Some Flux State vectors to randomly pick from
    printf("Setting up Flux State Vectors...\n");
    float *flux_states = (float *)malloc(N_flux_states * I.egroups * sizeof(float));
    int N_flux_states = 10000;
    assert(I.segments >= N_flux_states);

    #pragma omp target data map(to: flux_states[0:N_flux_states * I.egroups])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < N_flux_states; i++) {
            init_flux_states(flux_states, N_flux_states, I, RNG_states);
        }
    }

    printf("Initialization Complete.\n");
    border_print();
    center_print("SIMULATION", 79);
    border_print();
    printf("Attenuating fluxes across segments...\n");

    // Run Simulation Kernel Loop
    double start_time = omp_get_wtime();
    #pragma omp target data map(to: sources_d[0:I.source_3D_regions], SA_d, table_d, RNG_states[0:I.streams], flux_states[0:N_flux_states * I.egroups]) \
                            map(from: flux_states[0:N_flux_states * I.egroups])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < I.segments / I.seg_per_thread; i++) {
            run_kernel(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states);
        }
    }
    double end_time = omp_get_wtime();

    float *host_flux_states = (float *)malloc(N_flux_states * I.egroups * sizeof(float));
    memcpy(host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float));

    printf("Simulation Complete.\n");

    border_print();
    center_print("RESULTS SUMMARY", 79);
    border_print();

    double time = end_time - start_time;
    double tpi = (time / (double)I.segments / (double)I.egroups) * 1.0e9;
    printf("%-25s%.3f seconds\n", "Runtime:", time);
    printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
    border_print();

    free(RNG_states);
    free(flux_states);
    free(host_flux_states);

    return 0;
}