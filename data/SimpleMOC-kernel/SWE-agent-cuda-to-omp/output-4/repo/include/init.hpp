
#include <omp.h>
#include <curand_kernel.h>
#include "SimpleMOC-kernel_header.h"

void setup_kernel(curandState *state, Input I) {
    #pragma omp target teams distribute parallel for
    for (int threadId = 0; threadId < I.streams; threadId++) {
        curand_init(1234, threadId, 0, &state[threadId]);
    }
}

// Initialize global flux states to random numbers on device
void init_flux_states(float *flux_states, int N_flux_states, Input I, curandState *state) {
    #pragma omp target teams distribute parallel for
    for (int blockId = 0; blockId < N_flux_states; blockId++) {
        // Assign RNG state
        curandState *localState = &state[blockId % I.streams];

        if (omp_get_thread_num() == 0) {
            for (int i = 0; i < I.egroups; i++) {
                flux_states[blockId + i] = curand_uniform(localState);
            }
        }
    }
}

Input set_default_input(void) {
    Input I;
    I.source_2D_regions = 5000;
    I.coarse_axial_intervals = 27;
    I.fine_axial_intervals = 5;
    I.decomp_assemblies_ax = 20; // Number of subdomains per assembly axially
    I.segments = 50000000;
    I.egroups = 128;
    I.streams = 10000;
    I.seg_per_thread = 100;
    return I;
}

double mem_estimate(Input I) {
    size_t nbytes = 0;
    nbytes += I.source_3D_regions * sizeof(Source);
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    nbytes += N_fine * sizeof(float);
    nbytes += N_fine * sizeof(float);
    long N_sigT = I.source_3D_regions * I.egroups;
    nbytes += N_sigT * sizeof(float);
    return (double)nbytes / 1024.0 / 1024.0;
}

Source *initialize_sources(Input I, Source_Arrays *SA) {
    Source *sources = (Source *)malloc(I.source_3D_regions * sizeof(Source));
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    SA->fine_source_arr = (float *)malloc(N_fine * sizeof(float));
    for (int i = 0; i < I.source_3D_regions; i++) {
        sources[i].fine_source_id = i * I.fine_axial_intervals * I.egroups;
    }
    SA->fine_flux_arr = (float *)malloc(N_fine * sizeof(float));
    for (int i = 0; i < I.source_3D_regions; i++) {
        sources[i].fine_flux_id = i * I.fine_axial_intervals * I.egroups;
    }
    long N_sigT = I.source_3D_regions * I.egroups;
    SA->sigT_arr = (float *)malloc(N_sigT * sizeof(float));
    for (int i = 0; i < I.source_3D_regions; i++) {
        sources[i].sigT_id = i * I.egroups;
    }
    for (long i = 0; i < N_fine; i++) {
        SA->fine_source_arr[i] = (float)rand() / RAND_MAX;
        SA->fine_flux_arr[i] = (float)rand() / RAND_MAX;
    }
    for (int i = 0; i < N_sigT; i++) {
        SA->sigT_arr[i] = (float)rand() / RAND_MAX;
    }
    return sources;
}