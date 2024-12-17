#include "SimpleMOC-kernel_header.h"

void run_kernel(Input I, Source *S, Source_Arrays SA, Table *table, curandState *state,
                float *state_fluxes, int N_state_fluxes) {
    #pragma omp offload target(cuda)
    __global__ void kernel() {
        int blockId = blockIdx.y * gridDim.x + blockIdx.x;
        if (blockId >= I.segments / I.seg_per_thread)
            return;

        // Assign RNG state
        curandState *localState = &state[blockId % I.streams];

        blockId *= I.seg_per_thread;
        blockId--;

        int g = threadIdx.x; // Each energy group (g) is one thread in a block

        float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral;
        float tally;

        extern __shared__ int shm[];
        int *state_flux_id = &shm[0];
        int *QSR_id = &shm[I.seg_per_thread];
        int *FAI_id = &shm[I.seg_per_thread * 2];

        if (threadIdx.x == 0) {
            for (int i = 0; i < I.seg_per_thread; i++) {
                state_flux_id[i] = curand(localState) % N_state_fluxes;
                QSR_id[i] = curand(localState) % I.source_3D_regions;
                FAI_id[i] = curand(localState) % I.fine_axial_intervals;
            }
        }

#pragma omp offload target(cuda) is_device_ptr(S, SA)
        __syncthreads();

        for (int i = 0; i < I.seg_per_thread; i++) {
            blockId++;

            float *state_flux = &state_fluxes[state_flux_id[i]];

#pragma omp offload target(cuda) is_device_ptr(table)
            __syncthreads();

            // Attenuate Segment
            // ...

            // Load neighboring sources
            if (FAI_id[i] == 0) {
                // ...
            } else if (FAI_id[i] == I.fine_axial_intervals - 1) {
                // ...
            } else {
                // ...
            }

            sigT = __ldg(&SA.sigT_arr[S[QSR_id[i]].sigT_id + g]);

            tau = sigT * ds;
            sigT2 = sigT * sigT;

            #ifdef TABLE
            interpolateTable(table, tau, &expVal);
            #else
            expVal = 1.f - expf(-tau); // EXP function is fater than table lookup
            #endif

            reuse = tau * (tau - 2.f) + 2.f * expVal / (sigT * sigT2);

            flux_integral = (q0 * tau + (sigT * __ldg(&state_flux[g]) - q0)
                             * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2
                            * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal)
                            / (3.f * sigT2 * sigT2);

            tally = weight * flux_integral;

            // SHOULD BE ATOMIC HERE!
            atomicAdd(&FSR_flux[g], (float)tally);

            t1 = q0 * expVal / sigT;
            t2 = q1 * mu * (tau - expVal) / sigT2;
            t3 = q2 * mu2 * reuse;
            t4 = state_flux[g] * (1.f - expVal);
            state_flux[g] = t1 + t2 + t3 + t4;
        }
    }

#pragma omp target teams distribute parallel for
    kernel<<<I.streams / 100 + 1, 100>>>(I, S, SA, table, state, state_fluxes, N_state_fluxes);
}