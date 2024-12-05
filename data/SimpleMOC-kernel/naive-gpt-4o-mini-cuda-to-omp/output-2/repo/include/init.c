#include "SimpleMOC-kernel_header.h"

#pragma omp declare target
void setup_kernel(curandState *state, Input I)
{
    int threadId = omp_get_thread_num();
    if (threadId >= I.streams)
        return;
    curand_init(1234, threadId, 0, &state[threadId]);
}

void init_flux_states(float *flux_states, int N_flux_states, Input I, curandState *state)
{
    #pragma omp target teams distribute parallel for
    for (int blockId = 0; blockId < N_flux_states; blockId++) {
        // Assign RNG state
        curandState localState = state[blockId % I.streams];

        // Initialize flux states
        for (int g = 0; g < I.egroups; g++) {
            flux_states[blockId + g] = curand_uniform(&localState);
        }
    }
}

Input set_default_input(void)
{
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

double mem_estimate(Input I)
{
    size_t nbytes = 0;

    // Sources Array
    nbytes += I.source_3D_regions * sizeof(Source);

    // Fine Source Data
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    nbytes += N_fine * sizeof(float);

    // Fine Flux Data
    nbytes += N_fine * sizeof(float);

    // SigT Data
    long N_sigT = I.source_3D_regions * I.egroups;
    nbytes += N_sigT * sizeof(float);

    // Return MB
    return (double)nbytes / 1024.0 / 1024.0;
}

Source *initialize_sources(Input I, Source_Arrays *SA)
{
    // Source Data Structure Allocation
    Source *sources = (Source *)malloc(I.source_3D_regions * sizeof(Source));

    // Allocate Fine Source Data
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    SA->fine_source_arr = (float *)malloc(N_fine * sizeof(float));
    for (int i = 0; i < I.source_3D_regions; i++)
        sources[i].fine_source_id = i * I.fine_axial_intervals * I.egroups;

    // Allocate Fine Flux Data
    SA->fine_flux_arr = (float *)malloc(N_fine * sizeof(float));
    for (int i = 0; i < I.source_3D_regions; i++)
        sources[i].fine_flux_id = i * I.fine_axial_intervals * I.egroups;

    // Allocate SigT Data
    long N_sigT = I.source_3D_regions * I.egroups;
    SA->sigT_arr = (float *)malloc(N_sigT * sizeof(float));
    for (int i = 0; i < I.source_3D_regions; i++)
        sources[i].sigT_id = i * I.egroups;

    // Initialize fine source and flux to random numbers
    for (long i = 0; i < N_fine; i++)
    {
        SA->fine_source_arr[i] = (float)rand() / RAND_MAX;
        SA->fine_flux_arr[i] = (float)rand() / RAND_MAX;
    }

    // Initialize SigT Values
    for (int i = 0; i < N_sigT; i++)
        SA->sigT_arr[i] = (float)rand() / RAND_MAX;

    return sources;
}

Source *initialize_device_sources(Input I, Source_Arrays *SA_h, Source_Arrays *SA_d, Source *sources_h)
{
    // Allocate & Copy Fine Source Data
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    #pragma omp target enter data map(to: SA_d->fine_source_arr[0:N_fine])
    memcpy(SA_d->fine_source_arr, SA_h->fine_source_arr, N_fine * sizeof(float));

    // Allocate & Copy Fine Flux Data
    #pragma omp target enter data map(to: SA_d->fine_flux_arr[0:N_fine])
    memcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr, N_fine * sizeof(float));

    // Allocate & Copy SigT Data
    long N_sigT = I.source_3D_regions * I.egroups;
    #pragma omp target enter data map(to: SA_d->sigT_arr[0:N_sigT])
    memcpy(SA_d->sigT_arr, SA_h->sigT_arr, N_sigT * sizeof(float));

    // Allocate & Copy Source Array Data
    Source *sources_d;
    #pragma omp target enter data map(to: sources_d[0:I.source_3D_regions])
    memcpy(sources_d, sources_h, I.source_3D_regions * sizeof(Source));

    return sources_d;
}
#pragma omp end declare target