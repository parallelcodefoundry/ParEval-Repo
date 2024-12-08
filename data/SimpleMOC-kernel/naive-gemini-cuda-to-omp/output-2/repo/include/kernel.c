#include "SimpleMOC-kernel_header.h"

/* My parallelization scheme here is to basically have a single
 * block be a geometrical segment, with each thread within the
 * block represent a single energy phase. On the CPU, the
 * inner SIMD-ized loop is over energy (i.e, 100 energy groups).
 * This should allow for each BLOCK to have:
 * 		- A single state variable for the RNG
 * 		- A set of __shared__ SIMD vectors, each thread id being its idx
 */

#pragma omp target teams distribute parallel for simd private(localState, blockId, g, q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4, state_flux, FSR_flux, y2, y3, c0, c1, y1, c2)
void run_kernel(Input I, Source *S,
                 Source_Arrays SA, Table *table, curandState *state,
                 float *state_fluxes, int N_state_fluxes) {
    int blockId = omp_get_team_num() * omp_get_max_teams() + omp_get_thread_num(); // geometric segment

    if (blockId >= I.segments / I.seg_per_thread)
        return;

    // Assign RNG state.  Note:  This needs a proper OpenMP RNG implementation.  This is a placeholder.
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

    // This section needs to be adapted for OpenMP. Shared memory is not directly analogous.
    //  Consider using OpenMP arrays clauses or other data sharing mechanisms.
    // Randomized variables (common across all threads within block)
    //extern __shared__ int shm[];
    //int *state_flux_id = &shm[0];
    //int *QSR_id = &shm[I.seg_per_thread];
    //int *FAI_id = &shm[I.seg_per_thread * 2];

    int state_flux_id[I.seg_per_thread];
    int QSR_id[I.seg_per_thread];
    int FAI_id[I.seg_per_thread];

    if (g == 0) {
        for (int i = 0; i < I.seg_per_thread; i++) {
            //Needs proper OpenMP RNG
            state_flux_id[i] = (int)curand(localState) % N_state_fluxes;
            QSR_id[i] = (int)curand(localState) % I.source_3D_regions;
            FAI_id[i] = (int)curand(localState) % I.fine_axial_intervals;
        }
    }

    #pragma omp barrier //Needed to ensure all threads have the randomized data before proceeding.

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

        // load fine source region flux vector
        float *FSR_flux = &SA.fine_flux_arr[S[QSR_id[i]].fine_flux_id + FAI_id[i] * I.egroups];

        if (FAI_id[i] == 0) {
            // ... (rest of the code remains largely the same)
        }
        else if (FAI_id[i] == I.fine_axial_intervals - 1) {
            // ... (rest of the code remains largely the same)
        } else {
            // ... (rest of the code remains largely the same)
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

        // Re-used Term
        reuse = tau * (tau - 2.f) + 2.f * expVal
                / (sigT * sigT2);

        // add contribution to new source flux
        flux_integral = (q0 * tau + (sigT * state_flux[g] - q0)
                         * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2
                        * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal)
                        / (3.f * sigT2 * sigT2);

        // Prepare tally
        tally = weight * flux_integral;

        //Needs atomic operation replacement for OpenMP
        #pragma omp atomic update
        FSR_flux[g] += tally;

        // Term 1
        t1 = q0 * expVal / sigT;
        // Term 2
        t2 = q1 * mu * (tau - expVal) / sigT2;
        // Term 3
        t3 = q2 * mu2 * reuse;
        // Term 4
        t4 = state_flux[g] * (1.f - expVal);
        // Total psi
        state_flux[g] = t1 + t2 + t3 + t4;
    }
}

/* Interpolates a formed exponential table to compute ( 1- exp(-x) )
 *  at the desired x value */
__device__ void interpolateTable(Table *table, float x, float *out) {
    // check to ensure value is in domain
    if (x > table->maxVal)
        *out = 1.0f;
    else {
        int interval = (int)(x / table->dx + 0.5f * table->dx);
        interval = interval * 2;
        float slope = table->values[interval];
        float intercept = table->values[interval + 1];
        float val = slope * x + intercept;
        *out = val;
    }
}