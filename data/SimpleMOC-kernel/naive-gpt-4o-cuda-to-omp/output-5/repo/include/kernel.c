#include "SimpleMOC-kernel_header.h"
#include <omp.h>

/* My parallelization scheme here is to basically have a single
 * block be a geometrical segment, with each thread within the
 * block represent a single energy phase. On the CPU, the
 * inner SIMD-ized loop is over energy (i.e, 100 energy groups).
 * This should allow for each BLOCK to have:
 * 		- A single state variable for the RNG
 * 		- A set of shared SIMD vectors, each thread id being its idx
 */

void run_kernel(Input I, Source *S, Source_Arrays SA, Table *table, unsigned long long seed, float *state_fluxes, int N_state_fluxes) {
    int num_segments = I.segments / I.seg_per_thread;

    #pragma omp target teams distribute parallel for collapse(2) map(to: S[0:I.source_3D_regions], SA, table[0:1]) map(tofrom: state_fluxes[0:N_state_fluxes * I.egroups])
    for (int blockIdx_y = 0; blockIdx_y < num_segments; ++blockIdx_y) {
        for (int blockIdx_x = 0; blockIdx_x < num_segments; ++blockIdx_x) {
            int blockId = blockIdx_y * num_segments + blockIdx_x;

            if (blockId >= num_segments)
                continue;

            unsigned int localState = seed + blockId;

            blockId *= I.seg_per_thread;
            blockId--;

            #pragma omp parallel
            {
                int g = omp_get_thread_num(); // Each energy group (g) is one thread in a block

                // Thread Local (i.e., specific to E group) variables
                float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4;

                // Randomized variables (common across all threads within block)
                int state_flux_id[I.seg_per_thread];
                int QSR_id[I.seg_per_thread];
                int FAI_id[I.seg_per_thread];

                #pragma omp single
                {
                    for (int i = 0; i < I.seg_per_thread; i++) {
                        state_flux_id[i] = localState % N_state_fluxes;
                        QSR_id[i] = localState % I.source_3D_regions;
                        FAI_id[i] = localState % I.fine_axial_intervals;
                    }
                }

                #pragma omp barrier

                for (int i = 0; i < I.seg_per_thread; i++) {
                    blockId++;

                    float *state_flux = &state_fluxes[state_flux_id[i] * I.egroups];

                    #pragma omp barrier

                    //////////////////////////////////////////////////////////
                    // Attenuate Segment
                    //////////////////////////////////////////////////////////

                    // Some placeholder constants
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
                        float y2 = f2[g];
                        float y3 = f3[g];

                        float c0 = y2;
                        float c1 = (y3 - y2) / dz;

                        q0 = c0 + c1 * zin;
                        q1 = c1;
                        q2 = 0;
                    } else if (FAI_id[i] == I.fine_axial_intervals - 1) {
                        float *f1 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] - 1) * egroups];
                        float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
                        float y1 = f1[g];
                        float y2 = f2[g];

                        float c0 = y2;
                        float c1 = (y2 - y1) / dz;

                        q0 = c0 + c1 * zin;
                        q1 = c1;
                        q2 = 0;
                    } else {
                        float *f1 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] - 1) * egroups];
                        float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
                        float *f3 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] + 1) * egroups];
                        float y1 = f1[g];
                        float y2 = f2[g];
                        float y3 = f3[g];

                        float c0 = y2;
                        float c1 = (y1 - y3) / (2.f * dz);
                        float c2 = (y1 - 2.f * y2 + y3) / (2.f * dz * dz);

                        q0 = c0 + c1 * zin + c2 * zin * zin;
                        q1 = c1 + 2.f * c2 * zin;
                        q2 = c2;
                    }

                    sigT = SA.sigT_arr[S[QSR_id[i]].sigT_id + g];

                    tau = sigT * ds;
                    sigT2 = sigT * sigT;

                    #ifdef TABLE
                    interpolateTable(table, tau, &expVal);
                    #else
                    expVal = 1.f - expf(-tau);
                    #endif

                    reuse = tau * (tau - 2.f) + 2.f * expVal / (sigT * sigT2);

                    flux_integral = (q0 * tau + (sigT * state_flux[g] - q0) * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal) / (3.f * sigT2 * sigT2);

                    tally = weight * flux_integral;

                    #pragma omp atomic
                    FSR_flux[g] += tally;

                    t1 = q0 * expVal / sigT;
                    t2 = q1 * mu * (tau - expVal) / sigT2;
                    t3 = q2 * mu2 * reuse;
                    t4 = state_flux[g] * (1.f - expVal);
                    state_flux[g] = t1 + t2 + t3 + t4;
                }
            }
        }
    }
}

/* Interpolates a formed exponential table to compute ( 1- exp(-x) )
 *  at the desired x value */
void interpolateTable(Table *table, float x, float *out) {
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