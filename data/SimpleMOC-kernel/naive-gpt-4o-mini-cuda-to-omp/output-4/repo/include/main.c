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
    
    // Offload initialization of sources to the device
    #pragma omp target data map(to: I, SA_h, sources_h) map(from: SA_d)
    {
        initialize_device_sources(I, &SA_h, &SA_d, sources_h);
    }

    // Build Exponential Table
    Table *table_d = NULL;
    #ifdef TABLE
    printf("Building Exponential Table...\n");
    Table table = buildExponentialTable();
    #pragma omp target
    {
        table_d = (Table *)omp_target_alloc(sizeof(Table), omp_get_default_device());
        #pragma omp target enter data map(to: table) map(to: table_d)
        {
            *table_d = table;
        }
    }
    #endif

    // Setup OpenMP Offload
    printf("Setting up OpenMP Offload...\n");
    #pragma omp target
    {
        // Setup RNG states on device
        curandState *RNG_states;
        #pragma omp target
        {
            RNG_states = (curandState *)omp_target_alloc(I.streams * sizeof(curandState), omp_get_default_device());
            setup_kernel<<<(I.streams + 255) / 256, 256>>>(RNG_states, I);
        }
        #pragma omp target update to(RNG_states)

        // Allocate Some Flux State vectors to randomly pick from
        printf("Setting up Flux State Vectors...\n");
        float *flux_states;
        int N_flux_states = 10000;
        assert(I.segments >= N_flux_states);
        #pragma omp target
        {
            flux_states = (float *)omp_target_alloc(N_flux_states * I.egroups * sizeof(float), omp_get_default_device());
            init_flux_states<<<(I.segments + 255) / 256, 256>>>(flux_states, N_flux_states, I, RNG_states);
        }

        printf("Initialization Complete.\n");
        border_print();
        center_print("SIMULATION", 79);
        border_print();
        printf("Attenuating fluxes across segments...\n");

        // OpenMP Offload kernel call
        float *host_flux_states = (float *)malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target
        {
            run_kernel<<<(I.segments / I.seg_per_thread + 255) / 256, 256>>>(I, sources_d, SA_d, table_d,
                RNG_states, flux_states, N_flux_states);
        }

        // Copy results back to host
        #pragma omp target update from(flux_states)
        cudaMemcpy(host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float), cudaMemcpyDeviceToHost);

        printf("Simulation Complete.\n");

        border_print();
        center_print("RESULTS SUMMARY", 79);
        border_print();

        double tpi = ((double)(time / 1000.0) /
            (double)I.segments / (double)I.egroups) * 1.0e9;
        printf("%-25s%.3f seconds\n", "Runtime:", time / 1000.0);
        printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
        border_print();
    }

    return 0;
}