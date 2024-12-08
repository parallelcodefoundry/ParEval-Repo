#include "SimpleMOC-kernel_header.h"

/* My parallelization scheme here is to basically have a single
 * block be a geometrical segment, with each thread within the
 * block represent a single energy phase. On the CPU, the
 * inner SIMD-ized loop is over energy (i.e, 100 energy groups).
 * This should allow for each BLOCK to have:
 * 		- A single state variable for the RNG
 * 		- A set of __shared__ SIMD vectors, each thread id being its idx
 */

#pragma omp target teams distribute parallel for simd
void run_kernel(Input I, Source *S,
                 Source_Arrays SA, Table *table, curandState *state,
                 float *state_fluxes, int N_state_fluxes)
{
    int blockId = omp_get_team_num() * omp_get_max_teams() + omp_get_thread_num(); // geometric segment

    if (blockId >= I.segments / I.seg_per_thread)
        return;

    // Assign RNG state.  Note:  Need a proper way to manage RNG states in OpenMP offloading. This is a placeholder.
    curandState *localState = &state[blockId % I.streams];

    blockId *= I.seg_per_thread;
    blockId--;

    int g = omp_get_thread_num(); // Each energy group (g) is one thread in a block

    // Thread Local (i.e., specific to E group) variables
    // Similar to SIMD vectors in CPU code
    float q0;
    float q1;
    float q2;
    float sigT;
    float tau;
    float sigT2;
    float expVal;
    float reuse;
    float flux_integral;
    float tally;
    float t1;
    float t2;
    float t3;
    float t4;

    // Randomized variables (common across all thread within block)
    // Shared memory is not directly supported in OpenMP offloading in the same way as CUDA.  We'll use private variables instead.
    int state_flux_id[I.seg_per_thread];
    int QSR_id[I.seg_per_thread];
    int FAI_id[I.seg_per_thread];

    #pragma omp simd
    for (int i = 0; i < I.seg_per_thread; i++) {
        state_flux_id[i] = curand(&localState[i]) % N_state_fluxes; //Note: Modified to use localState array correctly.
        QSR_id[i] = curand(&localState[i]) % I.source_3D_regions; //Note: Modified to use localState array correctly.
        FAI_id[i] = curand(&localState[i]) % I.fine_axial_intervals; //Note: Modified to use localState array correctly.
    }


    for (int i = 0; i < I.seg_per_thread; i++) {
        blockId++;

        float *state_flux = &state_fluxes[state_flux_id[i]];

        //////////////////////////////////////////////////////////
        // Attenuate Segment
        //////////////////////////////////////////////////////////

        // Some placeholder constants - In the full app some of these are
        // calculated based off position in geometry. This treatment
        // shaves off a few FLOPS, but is not significant compared to the
        // rest of the function.
        float dz = 0.1f;
        float zin = 0.3f;
        float weight = 0.5f;
        float mu = 0.9f;
        float mu2 = 0.3f;
        float ds = 0.7f;

        const int egroups = I.egroups;

        // load fine source region flux vector
        float *FSR_flux = &SA.fine_flux_arr[S[QSR_id[i]].fine_flux_id + FAI_id[i] * egroups];

        //The following if-else if-else block needs to be carefully optimized for OpenMP offloading.  SIMDization may help, but memory access patterns need examination.
        if (FAI_id[i] == 0) {
            // ... (code remains largely the same) ...
        } else if (FAI_id[i] == I.fine_axial_intervals - 1) {
            // ... (code remains largely the same) ...
        } else {
            // ... (code remains largely the same) ...
        }

        // load total cross section
        sigT = SA.sigT_arr[S[QSR_id[i]].sigT_id + g];

        // calculate common values for efficiency
        tau = sigT * ds;
        sigT2 = sigT * sigT;

        #ifdef TABLE
        interpolateTable(table, tau, &expVal);
        #else
        expVal = 1.f - expf(-tau); // EXP function is faster than table lookup
        #endif

        // Flux Integral
        // ... (code remains largely the same) ...
        // SHOULD BE ATOMIC HERE!  OpenMP provides atomic operations.
        #pragma omp atomic
        FSR_flux[g] += tally;

        // ... (code remains largely the same) ...
    }
}

/* Interpolates a formed exponential table to compute ( 1- exp(-x) )
 *  at the desired x value */
__device__ void interpolateTable(Table *table, float x, float *out) {
    // ... (code remains the same) ...
}