#include "SimpleMOC-kernel_header.h"
#include <omp.h>

__attribute__((offload( target : omp_target_device))) 
void run_kernel(Input I, Source *  S,
	Source_Arrays SA, Table *  table, curandState *  state,
	float *  state_fluxes, int N_state_fluxes);

int main() {
    // ... (rest of the code remains the same)

    // Allocate device memory
    float * flux_states;
    int N_flux_states = 10000;

    #pragma omp offload target(omp_target_device)
    {
        CUDA_CALL(cudaMalloc((void **) &flux_states, N_flux_states * I.egroups * sizeof(float)));
    }

    init_flux_states<<< blocks, I.egroups >>> ( flux_states, N_flux_states, I, RNG_states );

    // ... (rest of the code remains the same)

    run_kernel<<< blocks_k, I.egroups, I.seg_per_thread * 3 *sizeof(int) >>> (I, sources_d, SA_d, table_d, 
        RNG_states, flux_states, N_flux_states);

    #pragma omp offload target(omp_target_device)
    {
        CUDA_CALL(cudaDeviceSynchronize());
    }

    // ... (rest of the code remains the same)

    return 0;
}

__attribute__((offload( target : omp_target_device))) 
void run_kernel(Input I, Source *  S,
	Source_Arrays SA, Table *  table, curandState *  state,
	float *  state_fluxes, int N_state_fluxes)
{
    // ... (rest of the code remains the same)

}