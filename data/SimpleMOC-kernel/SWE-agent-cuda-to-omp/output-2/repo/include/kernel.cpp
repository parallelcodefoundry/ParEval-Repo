
#include <omp.h>
#include "SimpleMOC-kernel_header.h"

void run_kernel(Input I, Source *S, Source_Arrays SA, Table *table, curandState *state, float *state_fluxes, int N_state_fluxes) {
    #pragma omp target teams distribute parallel for
    for (int blockId = 0; blockId < I.segments / I.seg_per_thread; blockId++) {
        // Assign RNG state
        curandState *localState = &state[blockId % I.streams];

        blockId *= I.seg_per_thread;
        blockId--;

        int g = omp_get_thread_num(); // Each energy group (g) is one thread in a block

        // Thread Local variables
        float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4;

        // Randomized variables (common across all threads within block)
        int state_flux_id[I.seg_per_thread];
        int QSR_id[I.seg_per_thread];
        int FAI_id[I.seg_per_thread];

        if (omp_get_thread_num() == 0) {
            for (int i = 0; i < I.seg_per_thread; i++) {
                state_flux_id[i] = curand(localState) % N_state_fluxes;
                QSR_id[i] = curand(localState) % I.source_3D_regions;
                FAI_id[i] = curand(localState) % I.fine_axial_intervals;
            }
        }

        #pragma omp barrier

        for (int i = 0; i < I.seg_per_thread; i++) {
            blockId++;
            float *state_flux = &state_fluxes[state_flux_id[i]];

            // Attenuate Segment
            float dz = 0.1f;
            float zin = 0.3f;
            float weight = 0.5f;
            float mu = 0.9f;
            float mu2 = 0.3f;
            float ds = 0.7f;

            const int egroups = I.egroups;

            // load fine source region flux vector
            float *FSR_flux = &SA.fine_flux_arr[S[QSR_id[i]].fine_flux_id + FAI_id[i] * egroups];

            if (FAI_id[i] == 0) {
                float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
                float *f3 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] + 1) * egroups];
                // cycle over energy groups
                // load neighboring sources
                float y2 = f2[g];
                float y3 = f3[g];
                // Additional computations...
            }
        }
    }
}