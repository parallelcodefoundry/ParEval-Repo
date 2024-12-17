#include "SimpleMOC-kernel_header.h"

// Note: The __global__ and __device__ keywords are not needed in OpenMP-Offload
// because they are replaced by OpenMP parallel constructs.

void setup_kernel(int num_streams, int num_threads_per_stream) {
    #pragma omp target teams distribute parallel for num_threads(num_threads_per_stream) offload
    for (int i = 0; i < num_streams; i++) {
        curand_init(1234, i * num_threads_per_stream, 0, &state[i]);
    }
}

// Initialize global flux states to random numbers on device
void init_flux_states(float *flux_states, int N_flux_states, int num_threads) {
    #pragma omp target data map(alloc: float[N_flux_states]) map(to: int[N_flux_states])
    {
        for (int i = 0; i < N_flux_states; i++) {
            flux_states[i] = curand_uniform(&state[blockIdx.x]);
        }
    }
}

// Gets I from user and sets defaults
Input set_default_input(void) {
    // This function does not need to be modified because it only accesses host variables.
}

// Returns a memory estimate (in MB) for the program's primary data structures
double mem_estimate(Input I) {
    // This function does not need to be modified because it only accesses host variables.
}

// Initialize source array data on device
void initialize_device_sources(Input I, Source_Arrays *SA_h, Source_Arrays *SA_d, Source *sources_h) {
    #pragma omp target teams distribute parallel for offload
    for (int i = 0; i < I.source_3D_regions; i++) {
        // Allocate & Copy Fine Source Data
        long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
        cudaMalloc((void **)&SA_d->fine_source_arr, N_fine * sizeof(float));
        cudaMemcpy(SA_d->fine_source_arr, SA_h->fine_source_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

        // Allocate & Copy Fine Flux Data
        cudaMalloc((void **)&SA_d->fine_flux_arr, N_fine * sizeof(float));
        cudaMemcpy(SA_d->fine_flux_arr, SA_h->fine_flux_arr, N_fine * sizeof(float), cudaMemcpyHostToDevice);

        // Allocate & Copy SigT Data
        long N_sigT = I.source_3D_regions * I.egroups;
        cudaMalloc((void **)&SA_d->sigT_arr, N_sigT * sizeof(float));
        cudaMemcpy(SA_d->sigT_arr, SA_h->sigT_arr, N_sigT * sizeof(float), cudaMemcpyHostToDevice);

        // Allocate & Copy Source Array Data
        Source *sources_d;
        cudaMalloc((void **)&sources_d, I.source_3D_regions * sizeof(Source));
        cudaMemcpy(sources_d, sources_h, I.source_3D_regions * sizeof(Source), cudaMemcpyHostToDevice);
    }
}

// Builds a table of exponential values for linear interpolation
Table buildExponentialTable(void) {
    // This function does not need to be modified because it only accesses host variables.
}