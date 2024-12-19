#include "SimpleMOC-kernel_header.h"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>

// Function to interpolate a formed exponential table to compute ( 1- exp(-x) )
//  at the desired x value
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

// Kernel function to attenuate segment
void run_kernel(Input I, Source *S, Source_Arrays SA, Table *table, float *state_fluxes, int N_state_fluxes) {
    int blockId = omp_get_thread_num(); // geometric segment
    int num_threads = omp_get_num_threads();

    if (blockId >= I.segments / I.seg_per_thread)
        return;

    blockId *= I.seg_per_thread;
    blockId--;

    int g = omp_get_thread_num() % I.egroups; // Each energy group (g) is one thread in a block

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

    // Randomized variables (common accross all thread within block)
    int *shm = (int *)malloc(I.seg_per_thread * 3 * sizeof(int));
    int *state_flux_id = &shm[0];
    int *QSR_id = &shm[I.seg_per_thread];
    int *FAI_id = &shm[I.seg_per_thread * 2];

    if (omp_get_thread_num() == 0) {
        for (int i = 0; i < I.seg_per_thread; i++) {
            state_flux_id[i] = rand() % N_state_fluxes;
            QSR_id[i] = rand() % I.source_3D_regions;
            FAI_id[i] = rand() % I.fine_axial_intervals;
        }
    }

    #pragma omp barrier

    for (int i = 0; i < I.seg_per_thread; i++) {
        blockId++;

        float *state_flux = &state_fluxes[state_flux_id[i]];

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

        if (FAI_id[i] == 0) {
            float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
            float *f3 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] + 1) * egroups];
            // cycle over energy groups
            // load neighboring sources
            float y2 = f2[g];
            float y3 = f3[g];

            // do linear "fitting"
            float c0 = y2;
            float c1 = (y3 - y2) / dz;

            // calculate q0, q1, q2
            q0 = c0 + c1 * zin;
            q1 = c1;
            q2 = 0;
        } else if (FAI_id[i] == I.fine_axial_intervals - 1) {
            float *f1 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] - 1) * egroups];
            float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
            // cycle over energy groups
            // load neighboring sources
            float y1 = f1[g];
            float y2 = f2[g];

            // do linear "fitting"
            float c0 = y2;
            float c1 = (y2 - y1) / dz;

            // calculate q0, q1, q2
            q0 = c0 + c1 * zin;
            q1 = c1;
            q2 = 0;
        } else {
            float *f1 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] - 1) * egroups];
            float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
            float *f3 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] + 1) * egroups];
            // cycle over energy groups
            // load neighboring sources
            float y1 = f1[g];
            float y2 = f2[g];
            float y3 = f3[g];

            // do quadratic "fitting"
            float c0 = y2;
            float c1 = (y1 - y3) / (2.0f * dz);
            float c2 = (y1 - 2.0f * y2 + y3) / (2.0f * dz * dz);

            // calculate q0, q1, q2
            q0 = c0 + c1 * zin + c2 * zin * zin;
            q1 = c1 + 2.0f * c2 * zin;
            q2 = c2;
        }

        // load total cross section
        sigT = SA.sigT_arr[S[QSR_id[i]].sigT_id + g];

        // calculate common values for efficiency
        tau = sigT * ds;
        sigT2 = sigT * sigT;

        #ifdef TABLE
        interpolateTable(table, tau, &expVal);
        #else
        expVal = 1.0f - expf(-tau); // EXP function is fater than table lookup
        #endif

        // Flux Integral

        // Re-used Term
        reuse = tau * (tau - 2.0f) + 2.0f * expVal / (sigT * sigT2);

        // add contribution to new source flux
        flux_integral = (q0 * tau + (sigT * state_flux[g] - q0) * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.0f) + 6.0f) - 6.0f * expVal) / (3.0f * sigT2 * sigT2);

        // Prepare tally
        tally = weight * flux_integral;

        // SHOULD BE ATOMIC HERE!
        // FSR_flux[g] += tally;
        #pragma omp atomic
        FSR_flux[g] += (float)tally;

        // Term 1
        t1 = q0 * expVal / sigT;
        // Term 2
        t2 = q1 * mu * (tau - expVal) / sigT2;
        // Term 3
        t3 = q2 * mu2 * reuse;
        // Term 4
        t4 = state_flux[g] * (1.0f - expVal);
        // Total psi
        state_flux[g] = t1 + t2 + t3 + t4;
    }

    free(shm);
}

int main() {
    // Initialize variables
    Input I;
    Source *S;
    Source_Arrays SA;
    Table *table;
    float *state_fluxes;
    int N_state_fluxes;

    // Set up OpenMP environment
    omp_set_num_threads(100);

    // Call kernel function
    #pragma omp parallel
    {
        run_kernel(I, S, SA, table, state_fluxes, N_state_fluxes);
    }

    return 0;
}