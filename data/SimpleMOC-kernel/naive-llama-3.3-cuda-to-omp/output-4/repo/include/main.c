#include "SimpleMOC-kernel_header.h"
#include <omp.h>

int main(int argc, char *argv[]) {
    int version = 4;

    srand(time(NULL));

    Input I = set_default_input();
    read_CLI(argc, argv, &I);

    logo(version);

    print_input_summary(I);

    printf("INITIALIZATION\n");
    // Build Source Data
    printf("Building Source Data Arrays...\n");
    Source_Arrays SA_h;
    Source *sources_h = initialize_sources(I, &SA_h);

    // Build Exponential Table
    Table *table_h = NULL;
#ifdef TABLE
    printf("Building Exponential Table...\n");
    Table table = buildExponentialTable();
    table_h = &table;
#endif

    // Allocate Some Flux State vectors to randomly pick from
    printf("Setting up Flux State Vectors...\n");
    float *flux_states_h;
    int N_flux_states = 10000;
    assert(I.segments >= N_flux_states);
    flux_states_h = (float *)malloc(N_flux_states * I.egroups * sizeof(float));

    // Initialize global flux states to random numbers on host
    for (int i = 0; i < N_flux_states * I.egroups; i++) {
        flux_states_h[i] = (float)rand() / RAND_MAX;
    }

    printf("Initialization Complete.\n");
    printf("SIMULATION\n");

    // Allocate device memory
    Source_Arrays SA_d;
    Source *sources_d;
    Table *table_d;
    float *flux_states_d;
    curandState *RNG_states_d;

    #pragma omp target map(to: I, sources_h[:I.source_3D_regions], SA_h) \
                        map(from: sources_d[0:I.source_3D_regions], SA_d)
    {
        // Allocate device memory
        cudaMalloc((void **)&sources_d, I.source_3D_regions * sizeof(Source));
        cudaMalloc((void **)&SA_d.fine_source_arr, I.source_3D_regions * I.fine_axial_intervals * I.egroups * sizeof(float));
        cudaMalloc((void **)&SA_d.fine_flux_arr, I.source_3D_regions * I.fine_axial_intervals * I.egroups * sizeof(float));
        cudaMalloc((void **)&SA_d.sigT_arr, I.source_3D_regions * I.egroups * sizeof(float));

        // Copy host data to device
        cudaMemcpy(sources_d, sources_h, I.source_3D_regions * sizeof(Source), cudaMemcpyHostToDevice);
        cudaMemcpy(SA_d.fine_source_arr, SA_h.fine_source_arr, I.source_3D_regions * I.fine_axial_intervals * I.egroups * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(SA_d.fine_flux_arr, SA_h.fine_flux_arr, I.source_3D_regions * I.fine_axial_intervals * I.egroups * sizeof(float), cudaMemcpyHostToDevice);
        cudaMemcpy(SA_d.sigT_arr, SA_h.sigT_arr, I.source_3D_regions * I.egroups * sizeof(float), cudaMemcpyHostToDevice);

        // Allocate and initialize RNG states
        cudaMalloc((void **)&RNG_states_d, I.streams * sizeof(curandState));
        setup_kernel<<<I.streams / 100 + 1, 100>>>(RNG_states_d, I);
        CudaCheckError();

        // Initialize flux states on device
        cudaMalloc((void **)&flux_states_d, N_flux_states * I.egroups * sizeof(float));
        init_flux_states<<<sqrt(I.segments), I.egroups>>>(flux_states_d, N_flux_states, I, RNG_states_d);

        // Run simulation kernel loop
        int n_blocks = sqrt(I.segments / I.seg_per_thread);
        dim3 blocks_k(n_blocks, n_blocks);
        if (blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread)
            blocks_k.x++;
        if (blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread)
            blocks_k.y++;
        assert(blocks_k.x * blocks_k.y >= I.segments / I.seg_per_thread);

        run_kernel<<<blocks_k, I.egroups, I.seg_per_thread * 3 * sizeof(int)>>>(I, sources_d, SA_d, table_h, RNG_states_d, flux_states_d, N_flux_states);
        CudaCheckError();
    }

    // Copy results from device to host
    float *host_flux_states = (float *)malloc(N_flux_states * I.egroups * sizeof(float));
    #pragma omp target map(from: flux_states_d[0:N_flux_states*I.egroups])
    {
        cudaMemcpy(host_flux_states, flux_states_d, N_flux_states * I.egroups * sizeof(float), cudaMemcpyDeviceToHost);
    }

    printf("Simulation Complete.\n");

    // Print results summary
    double tpi = ((double)(omp_get_wtime() / (I.segments * I.egroups))) * 1.0e9;
    printf("%-25s%.3f seconds\n", "Runtime:", omp_get_wtime());
    printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);

    return 0;
}