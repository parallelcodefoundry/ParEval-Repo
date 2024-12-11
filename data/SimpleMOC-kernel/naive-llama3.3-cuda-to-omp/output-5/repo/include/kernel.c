#include "SimpleMOC-kernel_header.h"
#include <omp.h>

// Interpolates a formed exponential table to compute ( 1- exp(-x) )
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

// My parallelization scheme here is to basically have a single
// block be a geometrical segment, with each thread within the
// block represent a single energy phase. On the CPU, the
// inner SIMD-ized loop is over energy (i.e, 100 energy groups).
// This should allow for each BLOCK to have:
// 		- A single state variable for the RNG
// 		- A set of __shared__ SIMD vectors, each thread id being its idx
void run_kernel(Input I, Source *S, Source_Arrays SA, Table *table, float *state_fluxes, int N_state_fluxes) {
    #pragma omp target teams distribute parallel for
    for (int blockId = 0; blockId < I.segments / I.seg_per_thread; blockId++) {
        // Assign RNG state
        int localState = blockId % I.streams;

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
        int state_flux_id[I.seg_per_thread];
        int QSR_id[I.seg_per_thread];
        int FAI_id[I.seg_per_thread];

        #pragma omp parallel for
        for (int i = 0; i < I.seg_per_thread; i++) {
            state_flux_id[i] = rand() % N_state_fluxes;
            QSR_id[i] = rand() % I.source_3D_regions;
            FAI_id[i] = rand() % I.fine_axial_intervals;
        }

        #pragma omp parallel for
        for (int i = 0; i < I.seg_per_thread; i++) {
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
                float y2 = f2[0]; // Assuming first element of the array for demonstration purposes
                float y3 = f3[0];

                // do linear "fitting"
                float c0 = y2;
                float c1 = (y3 - y2) / dz;

                // calculate q0, q1, q2
                q0 = c0 + c1 * zin;
                q1 = c1;
                q2 = 0;
            }
            else if (FAI_id[i] == I.fine_axial_intervals - 1) {
                float *f1 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] - 1) * egroups];
                float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
                // cycle over energy groups
                // load neighboring sources
                float y1 = f1[0]; // Assuming first element of the array for demonstration purposes
                float y2 = f2[0];

                // do linear "fitting"
                float c0 = y2;
                float c1 = (y2 - y1) / dz;

                // calculate q0, q1, q2
                q0 = c0 + c1 * zin;
                q1 = c1;
                q2 = 0;
            }
            else {
                float *f1 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] - 1) * egroups];
                float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
                float *f3 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] + 1) * egroups];
                // cycle over energy groups
                // load neighboring sources
                float y1 = f1[0]; // Assuming first element of the array for demonstration purposes
                float y2 = f2[0];
                float y3 = f3[0];

                // do quadratic "fitting"
                float c0 = y2;
                float c1 = (y1 - y3) / (2.f * dz);
                float c2 = (y1 - 2.f * y2 + y3) / (2.f * dz * dz);

                // calculate q0, q1, q2
                q0 = c0 + c1 * zin + c2 * zin * zin;
                q1 = c1 + 2.f * c2 * zin;
                q2 = c2;
            }

            // load total cross section
            sigT = SA.sigT_arr[S[QSR_id[i]].sigT_id];

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
            reuse = tau * (tau - 2.f) + 2.f * expVal / (sigT * sigT2);

            // add contribution to new source flux
            flux_integral = (q0 * tau + (sigT * state_fluxes[state_flux_id[i]]) - q0) * expVal / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal) / (3.f * sigT2 * sigT2);

            // Prepare tally
            tally = weight * flux_integral;

            // SHOULD BE ATOMIC HERE!
            //FSR_flux[g] += tally;
            FSR_flux[0] += tally; // Assuming first element of the array for demonstration purposes

            // Term 1
            t1 = q0 * expVal / sigT;
            // Term 2
            t2 = q1 * mu * (tau - expVal) / sigT2;
            // Term 3
            t3 = q2 * mu2 * reuse;
            // Term 4
            t4 = state_fluxes[state_flux_id[i]] * (1.f - expVal);
            // Total psi
            state_fluxes[state_flux_id[i]] = t1 + t2 + t3 + t4;
        }
    }
}