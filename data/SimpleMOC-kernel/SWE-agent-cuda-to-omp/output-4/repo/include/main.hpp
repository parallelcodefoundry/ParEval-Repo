
#include <omp.h>
#include <curand_kernel.h>
#include "SimpleMOC-kernel_header.h"

int main(int argc, char *argv[]) {
    int version = 4;
    srand(time(NULL));

    Input I = set_default_input();
    read_CLI(argc, argv, &I);

    // Calculate Number of 3D Source Regions
    I.source_3D_regions = (int) ceil((double)I.source_2D_regions * I.coarse_axial_intervals / I.decomp_assemblies_ax);

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
    // Allocate memory for table on device
    #endif

    // Setup OpenMP Offload
    printf("Setting up OpenMP Offload...\n");
    curandState *RNG_states;
    #pragma omp target data map(to: I) map(alloc: RNG_states)
    {
        RNG_states = (curandState *)malloc(I.streams * sizeof(curandState));
        setup_kernel(RNG_states, I);
    }

    // Allocate Some Flux State vectors to randomly pick from
    printf("Setting up Flux State Vectors...\n");
    float *flux_states;
    int N_flux_states = 10000;
    assert(I.segments >= N_flux_states);
    #pragma omp target data map(alloc: flux_states)
    {
        flux_states = (float *)malloc(N_flux_states * I.egroups * sizeof(float));
        init_flux_states(flux_states, N_flux_states, I, RNG_states);
    }

    printf("Initialization Complete.\n");
    border_print();
    center_print("SIMULATION", 79);
    border_print();

    printf("Attenuating fluxes across segments...\n");

    // Setup kernel call block parameters
    assert(I.segments % I.seg_per_thread == 0);
    int n_blocks = sqrt(I.segments / I.seg_per_thread);

    // Run Simulation Kernel Loop
    #pragma omp target teams distribute parallel for
    for (int blockId = 0; blockId < n_blocks; blockId++) {
        run_kernel(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states);
    }

    float *host_flux_states = (float *)malloc(N_flux_states * I.egroups * sizeof(float));
    // Copy results back to host
    return 0;
}