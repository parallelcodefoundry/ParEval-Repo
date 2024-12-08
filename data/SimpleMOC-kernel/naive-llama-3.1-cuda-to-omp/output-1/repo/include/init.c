#include <omp.h>
#include <cuda_runtime.h>

// Note: The following macros are used to indicate which functions should be offloaded.
#define OFFLOAD_FUNCTION __attribute__((offload( target : host, where : host)))

void OFFLOAD_FUNCTION initialize_host_sources(Input I, Source_Arrays *SA_h) {
    // Initialize fine source and flux to random numbers
    for (long i = 0; i < N_fine; i++) {
        SA_h->fine_source_arr[i] = (float) rand() / RAND_MAX;
        SA_h->fine_flux_arr[i] = (float) rand() / RAND_MAX;
    }

    // Initialize SigT Values
    for (int i = 0; i < N_sigT; i++) {
        SA_h->sigT_arr[i] = (float) rand() / RAND_MAX;
    }
}

void OFFLOAD_FUNCTION initialize_device_sources(Input I, Source_Arrays *SA_h, Source_Arrays *SA_d) {
    // Allocate & Copy Fine Source Data
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    cudaMalloc((void**) &SA_d->fine_source_arr, N_fine * sizeof(float));
    cudaMemcpy(SA_d->fine_source_arr, SA_h->fine_source_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

    // Allocate & Copy Fine Flux Data
    cudaMalloc((void**) &SA_d->fine_flux_arr, N_fine * sizeof(float));
    cudaMemcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

    // Allocate & Copy SigT Data
    long N_sigT = I.source_3D_regions * I.egroups;
    cudaMalloc((void**) &SA_d->sigT_arr, N_sigT * sizeof(float));
    cudaMemcpy(SA_d->sigT_arr, SA_h->sigT_arr, N_sigT * sizeof(float), cudaMemcpyHostToDevice);
}

// Note: The following functions will be offloaded to the device
void OFFLOAD_FUNCTION initialize_host_sources_offload(Input I, Source_Arrays *SA_h) {
    // Initialize fine source and flux to random numbers
    #pragma omp parallel for
    for (long i = 0; i < N_fine; i++) {
        SA_h->fine_source_arr[i] = (float) rand() / RAND_MAX;
        SA_h->fine_flux_arr[i] = (float) rand() / RAND_MAX;
    }

    // Initialize SigT Values
    #pragma omp parallel for
    for (int i = 0; i < N_sigT; i++) {
        SA_h->sigT_arr[i] = (float) rand() / RAND_MAX;
    }
}

void OFFLOAD_FUNCTION initialize_device_sources_offload(Input I, Source_Arrays *SA_h, Source_Arrays *SA_d) {
    // Allocate & Copy Fine Source Data
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    #pragma omp offload target(mic)
    cudaMalloc((void**) &SA_d->fine_source_arr, N_fine * sizeof(float));
    #pragma omp offload target(mic) in(SA_h) out(SA_d)
    cudaMemcpy(SA_d->fine_source_arr, SA_h->fine_source_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

    // Allocate & Copy Fine Flux Data
    #pragma omp offload target(mic)
    cudaMalloc((void**) &SA_d->fine_flux_arr, N_fine * sizeof(float));
    #pragma omp offload target(mic) in(SA_h) out(SA_d)
    cudaMemcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

    // Allocate & Copy SigT Data
    long N_sigT = I.source_3D_regions * I.egroups;
    #pragma omp offload target(mic)
    cudaMalloc((void**) &SA_d->sigT_arr, N_sigT * sizeof(float));
    #pragma omp offload target(mic) in(SA_h) out(SA_d)
    cudaMemcpy(SA_d->sigT_arr, SA_h->sigT_arr, N_sigT * sizeof(float), cudaMemcpyHostToDevice);
}

void buildExponentialTable() {
    // ... (no changes needed here)
}