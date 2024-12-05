#include "SimpleMOC-kernel_header.h"
#include <omp.h>

int main(int argc, char *argv[])
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
    Source *sources_h = initialize_sources(I, &SA_h);
    Source *sources_d = initialize_device_sources(I, &SA_h, &SA_d, sources_h);

    // Build Exponential Table
    Table *table_d = NULL;
#ifdef TABLE
    printf("Building Exponential Table...\n");
    Table table = buildExponentialTable();
    table_d = (Table *)malloc(sizeof(Table));
    *table_d = table;
#endif

    // Setup OpenMP Offload
    #pragma omp target enter data map(to: I, sources_d[0:I.source_3D_regions], SA_d)
    #pragma omp target enter data map(to: table_d) if(table_d != NULL)

    // Setup RNG on Device
    printf("Setting up RNG...\n");
    curandState *RNG_states;
    #pragma omp target device(0) map(to: I) map(alloc: RNG_states)
    {
        setup_kernel(RNG_states, I);
    }

    // Allocate Some Flux State vectors to randomly pick from
    printf("Setting up Flux State Vectors...\n");
    float *flux_states;
    int N_flux_states = 10000;
    assert(I.segments >= N_flux_states);
    #pragma omp target device(0) map(alloc: flux_states)
    {
        init_flux_states(flux_states, N_flux_states, I, RNG_states);
    }

    printf("Initialization Complete.\n");
    border_print();
    center_print("SIMULATION", 79);
    border_print();
    
    printf("Attenuating fluxes across segments...\n");

    // OpenMP timer variables
    double start_time = omp_get_wtime();

    // Setup kernel call block parameters
    assert(I.segments % I.seg_per_thread == 0);
    int n_blocks = sqrt(I.segments / I.seg_per_thread);
    dim3 blocks_k(n_blocks, n_blocks);
    if (blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread)
        blocks_k.x++;
    if (blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread)
        blocks_k.y++;
    assert(blocks_k.x * blocks_k.y >= I.segments / I.seg_per_thread);

    // Run Simulation Kernel Loop
    #pragma omp target teams distribute parallel for
    for (int blockId = 0; blockId < I.segments / I.seg_per_thread; blockId++) {
        run_kernel(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states);
    }

    double end_time = omp_get_wtime();
    float time = (float)(end_time - start_time) * 1000.0; // Convert to milliseconds

    float *host_flux_states = (float *)malloc(N_flux_states * I.egroups * sizeof(float));
    #pragma omp target exit data map(from: host_flux_states[0:N_flux_states * I.egroups])

    printf("Simulation Complete.\n");

    border_print();
    center_print("RESULTS SUMMARY", 79);
    border_print();

    double tpi = ((double)(time / 1000.0) /
                  (double)I.segments / (double)I.egroups) * 1.0e9;
    printf("%-25s%.3f seconds\n", "Runtime:", time / 1000.0);
    printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
    border_print();

    // Cleanup
    #pragma omp target exit data map(delete: sources_d, SA_d, table_d, RNG_states, flux_states)
    free(host_flux_states);

    return 0;
}