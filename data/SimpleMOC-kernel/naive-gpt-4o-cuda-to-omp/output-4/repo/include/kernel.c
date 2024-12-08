#include "SimpleMOC-kernel_header.h"
#include <omp.h>

// Function to interpolate a formed exponential table to compute (1 - exp(-x)) at the desired x value
void interpolateTable(Table *table, float x, float *out) {
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

void run_kernel(Input I, Source *S, Source_Arrays SA, Table *table, unsigned int *state, float *state_fluxes, int N_state_fluxes) {
    const int num_segments = I.segments / I.seg_per_thread;
    const int egroups = I.egroups;

    #pragma omp target teams distribute parallel for collapse(2) map(to: S[0:I.source_3D_regions], SA, table[0:1], state[0:I.streams]) map(tofrom: state_fluxes[0:N_state_fluxes * I.egroups])
    for (int blockId = 0; blockId < num_segments; ++blockId) {
        for (int i = 0; i < I.seg_per_thread; ++i) {
            int global_blockId = blockId * I.seg_per_thread + i;
            if (global_blockId >= I.segments) continue;

            // Assign RNG state
            unsigned int localState = state[blockId % I.streams];

            // Thread Local (i.e., specific to E group) variables
            float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4;

            // Randomized variables (common across all threads within block)
            int state_flux_id = rand_r(&localState) % N_state_fluxes;
            int QSR_id = rand_r(&localState) % I.source_3D_regions;
            int FAI_id = rand_r(&localState) % I.fine_axial_intervals;

            float *state_flux = &state_fluxes[state_flux_id * egroups];

            // Placeholder constants
            float dz = 0.1f;
            float zin = 0.3f;
            float weight = 0.5f;
            float mu = 0.9f;
            float mu2 = 0.3f;
            float ds = 0.7f;

            // load fine source region flux vector
            float *FSR_flux = &SA.fine_flux_arr[S[QSR_id].fine_flux_id + FAI_id * egroups];

            if (FAI_id == 0) {
                float *f2 = &SA.fine_source_arr[S[QSR_id].fine_source_id + (FAI_id) * egroups];
                float *f3 = &SA.fine_source_arr[S[QSR_id].fine_source_id + (FAI_id + 1) * egroups];
                float y2 = f2[0];
                float y3 = f3[0];
                float c0 = y2;
                float c1 = (y3 - y2) / dz;
                q0 = c0 + c1 * zin;
                q1 = c1;
                q2 = 0;
            } else if (FAI_id == I.fine_axial_intervals - 1) {
                float *f1 = &SA.fine_source_arr[S[QSR_id].fine_source_id + (FAI_id - 1) * egroups];
                float *f2 = &SA.fine_source_arr[S[QSR_id].fine_source_id + (FAI_id) * egroups];
                float y1 = f1[0];
                float y2 = f2[0];
                float c0 = y2;
                float c1 = (y2 - y1) / dz;
                q0 = c0 + c1 * zin;
                q1 = c1;
                q2 = 0;
            } else {
                float *f1 = &SA.fine_source_arr[S[QSR_id].fine_source_id + (FAI_id - 1) * egroups];
                float *f2 = &SA.fine_source_arr[S[QSR_id].fine_source_id + (FAI_id) * egroups];
                float *f3 = &SA.fine_source_arr[S[QSR_id].fine_source_id + (FAI_id + 1) * egroups];
                float y1 = f1[0];
                float y2 = f2[0];
                float y3 = f3[0];
                float c0 = y2;
                float c1 = (y1 - y3) / (2.f * dz);
                float c2 = (y1 - 2.f * y2 + y3) / (2.f * dz * dz);
                q0 = c0 + c1 * zin + c2 * zin * zin;
                q1 = c1 + 2.f * c2 * zin;
                q2 = c2;
            }

            // load total cross section
            sigT = SA.sigT_arr[S[QSR_id].sigT_id];

            // calculate common values for efficiency
            tau = sigT * ds;
            sigT2 = sigT * sigT;

            #ifdef TABLE
            interpolateTable(table, tau, &expVal);
            #else
            expVal = 1.f - expf(-tau);
            #endif

            // Flux Integral
            reuse = tau * (tau - 2.f) + 2.f * expVal / (sigT * sigT2);
            flux_integral = (q0 * tau + (sigT * state_flux[0] - q0) * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal) / (3.f * sigT2 * sigT2);
            tally = weight * flux_integral;

            #pragma omp atomic
            FSR_flux[0] += tally;

            t1 = q0 * expVal / sigT;
            t2 = q1 * mu * (tau - expVal) / sigT2;
            t3 = q2 * mu2 * reuse;
            t4 = state_flux[0] * (1.f - expVal);
            state_flux[0] = t1 + t2 + t3 + t4;
        }
    }
}