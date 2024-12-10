#include "SimpleMOC-kernel_header.h"
#include <omp.h>

int main(int argc, char *argv[]) {
    int version = 4;

    srand(time(NULL));

    Input I = set_default_input();
    read_CLI(argc, argv, &I);

    logo(version);

    print_input_summary(I);

    center_print("INITIALIZATION", 79);
    border_print();

    // Build Source Data
    printf("Building Source Data Arrays...\n");
    Source_Arrays SA_h;
    Source *sources_h = initialize_sources(I, &SA_h);

    // Build Exponential Table
    Table *table_d = NULL;
#ifdef TABLE
    printf("Building Exponential Table...\n");
    Table table = buildExponentialTable();
#endif

    // Setup OpenMP offload target
    int device_id = omp_get_default_device();
    if (device_id == -1) {
        fprintf(stderr, "No OpenMP target devices available.\n");
        return 1;
    }

#pragma omp target map(to: I)
    {
        // Allocate and initialize device data structures
        Source *sources_d;
        Source_Arrays SA_d;

        sources_d = (Source *)malloc(I.source_3D_regions * sizeof(Source));
        SA_d.fine_source_arr = (float *)malloc(I.source_3D_regions * I.fine_axial_intervals * I.egroups * sizeof(float));
        SA_d.fine_flux_arr = (float *)malloc(I.source_3D_regions * I.fine_axial_intervals * I.egroups * sizeof(float));
        SA_d.sigT_arr = (float *)malloc(I.source_3D_regions * I.egroups * sizeof(float));

        // Copy host data to device
        for (int i = 0; i < I.source_3D_regions; i++) {
            sources_d[i] = sources_h[i];
        }
        for (int i = 0; i < I.source_3D_regions * I.fine_axial_intervals * I.egroups; i++) {
            SA_d.fine_source_arr[i] = SA_h.fine_source_arr[i];
            SA_d.fine_flux_arr[i] = SA_h.fine_flux_arr[i];
        }
        for (int i = 0; i < I.source_3D_regions * I.egroups; i++) {
            SA_d.sigT_arr[i] = SA_h.sigT_arr[i];
        }

#ifdef TABLE
        // Allocate and initialize exponential table on device
        Table *table_d;
        table_d = (Table *)malloc(sizeof(Table));
        *table_d = table;
#endif

        printf("Initialization Complete.\n");
        border_print();
        center_print("SIMULATION", 79);
        border_print();

        float *flux_states;
        int N_flux_states = 10000;
        assert(I.segments >= N_flux_states);

        // Allocate and initialize flux state vectors on device
        flux_states = (float *)malloc(N_flux_states * I.egroups * sizeof(float));

#pragma omp target map(to: I, sources_d[0:I.source_3D_regions], SA_d) \
                         map(from: flux_states[0:N_flux_states * I.egroups])
        {
            // Initialize global flux states to random numbers on device
            for (int i = 0; i < N_flux_states; i++) {
                for (int j = 0; j < I.egroups; j++) {
                    flux_states[i * I.egroups + j] = (float)rand() / RAND_MAX;
                }
            }

#pragma omp target teams distribute map(to: I, sources_d[0:I.source_3D_regions], SA_d, table_d, flux_states[0:N_flux_states * I.egroups]) \
                                         map(from: flux_states[0:N_flux_states * I.egroups])
            {
                // Run simulation kernel loop
                for (int i = 0; i < I.segments / I.seg_per_thread; i++) {
                    int blockId = i;
                    // Assign RNG state
                    float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4;

                    for (int j = 0; j < I.seg_per_thread; j++) {
                        blockId++;

                        // Load fine source region flux vector
                        float *FSR_flux = &SA_d.fine_flux_arr[sources_d[blockId % I.source_3D_regions].fine_flux_id + (blockId / I.source_3D_regions) * I.egroups];

                        // Calculate q0, q1, q2
                        if ((blockId / I.source_3D_regions) == 0) {
                            float y2 = SA_d.fine_source_arr[sources_d[blockId % I.source_3D_regions].fine_source_id + (blockId / I.source_3D_regions) * I.egroups];
                            float y3 = SA_d.fine_source_arr[sources_d[blockId % I.source_3D_regions].fine_source_id + ((blockId / I.source_3D_regions) + 1) * I.egroups];

                            // do linear "fitting"
                            float c0 = y2;
                            float c1 = (y3 - y2) / 0.1f;

                            q0 = c0 + c1 * 0.3f;
                            q1 = c1;
                            q2 = 0;
                        } else if ((blockId / I.source_3D_regions) == I.fine_axial_intervals - 1) {
                            float y1 = SA_d.fine_source_arr[sources_d[blockId % I.source_3D_regions].fine_source_id + ((blockId / I.source_3D_regions) - 1) * I.egroups];
                            float y2 = SA_d.fine_source_arr[sources_d[blockId % I.source_3D_regions].fine_source_id + (blockId / I.source_3D_regions) * I.egroups];

                            // do linear "fitting"
                            float c0 = y2;
                            float c1 = (y2 - y1) / 0.1f;

                            q0 = c0 + c1 * 0.3f;
                            q1 = c1;
                            q2 = 0;
                        } else {
                            float y1 = SA_d.fine_source_arr[sources_d[blockId % I.source_3D_regions].fine_source_id + ((blockId / I.source_3D_regions) - 1) * I.egroups];
                            float y2 = SA_d.fine_source_arr[sources_d[blockId % I.source_3D_regions].fine_source_id + (blockId / I.source_3D_regions) * I.egroups];
                            float y3 = SA_d.fine_source_arr[sources_d[blockId % I.source_3D_regions].fine_source_id + ((blockId / I.source_3D_regions) + 1) * I.egroups];

                            // do quadratic "fitting"
                            float c0 = y2;
                            float c1 = (y1 - y3) / (2.f * 0.1f);
                            float c2 = (y1 - 2.f * y2 + y3) / (2.f * 0.1f * 0.1f);

                            q0 = c0 + c1 * 0.3f + c2 * 0.3f * 0.3f;
                            q1 = c1 + 2.f * c2 * 0.3f;
                            q2 = c2;
                        }

                        // Load total cross section
                        sigT = SA_d.sigT_arr[sources_d[blockId % I.source_3D_regions].sigT_id];

                        tau = sigT * 0.7f;
                        sigT2 = sigT * sigT;

#ifdef TABLE
                        interpolateTable(table_d, tau, &expVal);
#else
                        expVal = 1.f - expf(-tau);
#endif

                        reuse = tau * (tau - 2.f) + 2.f * expVal / (sigT * sigT2);

                        // Add contribution to new source flux
                        flux_integral = (q0 * tau + (sigT * flux_states[blockId % N_flux_states * I.egroups] - q0) * expVal) / sigT2 +
                                         q1 * 0.9f * reuse + q2 * 0.3f * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal) /
                                                                                     (3.f * sigT2 * sigT2);

                        // Prepare tally
                        float weight = 0.5f;
                        tally = weight * flux_integral;

                        FSR_flux[j] += tally;

                        t1 = q0 * expVal / sigT;
                        t2 = q1 * 0.9f * (tau - expVal) / sigT2;
                        t3 = q2 * 0.3f * reuse;
                        t4 = flux_states[blockId % N_flux_states * I.egroups] * (1.f - expVal);

                        flux_states[blockId % N_flux_states * I.egroups] = t1 + t2 + t3 + t4;
                    }
                }
            }

            printf("Simulation Complete.\n");

            border_print();
            center_print("RESULTS SUMMARY", 79);
            border_print();

            double time = omp_get_wtime();
            double tpi = ((double)(time) / (double)I.segments / (double)I.egroups) * 1.0e9;
            printf("%-25s%.3f seconds\n", "Runtime:", time);
            printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);

            border_print();
        }
    }

    return 0;
}