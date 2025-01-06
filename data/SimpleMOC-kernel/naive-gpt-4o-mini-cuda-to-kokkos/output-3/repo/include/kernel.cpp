#include "SimpleMOC-kernel_header.h"
#include <Kokkos_Core.hpp>

/* My parallelization scheme here is to basically have a single
 * team be a geometrical segment, with each thread within the
 * team represent a single energy phase. On the CPU, the
 * inner SIMD-ized loop is over energy (i.e., 100 energy groups).
 * This should allow for each TEAM to have:
 * 		- A single state variable for the RNG
 * 		- A set of Kokkos::View vectors, each thread id being its idx
 */

void run_kernel(Input I, Source *S, Source_Arrays SA, Table *table, curandState *state, float *state_fluxes, int N_state_fluxes) {
    Kokkos::parallel_for("run_kernel", Kokkos::TeamPolicy<>(I.segments / I.seg_per_thread, Kokkos::AUTO), KOKKOS_LAMBDA(const Kokkos::TeamPolicy<>::member_type &team) {
        int blockId = team.league_rank() * I.seg_per_thread; // geometric segment

        // Assign RNG state
        curandState *localState = &state[blockId % I.streams];

        int g = team.team_rank(); // Each energy group (g) is one thread in a team

        // Thread Local (i.e., specific to E group) variables
        float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4;

        // Randomized variables (common across all threads within team)
        Kokkos::View<int*> state_flux_id("state_flux_id", I.seg_per_thread);
        Kokkos::View<int*> QSR_id("QSR_id", I.seg_per_thread);
        Kokkos::View<int*> FAI_id("FAI_id", I.seg_per_thread);

        if (team.team_rank() == 0) {
            for (int i = 0; i < I.seg_per_thread; i++) {
                state_flux_id(i) = curand(localState) % N_state_fluxes;
                QSR_id(i) = curand(localState) % I.source_3D_regions;
                FAI_id(i) = curand(localState) % I.fine_axial_intervals;
            }
        }

        team.team_barrier();

        for (int i = 0; i < I.seg_per_thread; i++) {
            blockId++;

            float *state_flux = &state_fluxes[state_flux_id(i)];

            team.team_barrier();

            //////////////////////////////////////////////////////////
            // Attenuate Segment
            //////////////////////////////////////////////////////////

            float dz = 0.1f;
            float zin = 0.3f; 
            float weight = 0.5f;
            float mu = 0.9f;
            float mu2 = 0.3f;
            float ds = 0.7f;

            const int egroups = I.egroups;

            // load fine source region flux vector
            float *FSR_flux = &SA.fine_flux_arr[S[QSR_id(i)].fine_flux_id + FAI_id(i) * egroups];

            if (FAI_id(i) == 0) {
                float *f2 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i)) * egroups];
                float *f3 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i) + 1) * egroups];
                float y2 = f2[g];
                float y3 = f3[g];

                float c0 = y2;
                float c1 = (y3 - y2) / dz;

                q0 = c0 + c1 * zin;
                q1 = c1;
                q2 = 0;
            } else if (FAI_id(i) == I.fine_axial_intervals - 1) {
                float *f1 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i) - 1) * egroups];
                float *f2 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i)) * egroups];
                float y1 = f1[g];
                float y2 = f2[g];

                float c0 = y2;
                float c1 = (y2 - y1) / dz;

                q0 = c0 + c1 * zin;
                q1 = c1;
                q2 = 0;
            } else {
                float *f1 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i) - 1) * egroups];
                float *f2 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i)) * egroups];
                float *f3 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i) + 1) * egroups];
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

            // load total cross section
            sigT = SA.sigT_arr[S[QSR_id(i)].sigT_id + g];

            // calculate common values for efficiency
            tau = sigT * ds;
            sigT2 = sigT * sigT;

            float expVal;
            #ifdef TABLE
            interpolateTable(table, tau, &expVal);
            #else
            expVal = 1.f - expf(-tau); // EXP function is faster than table lookup
            #endif

            // Flux Integral
            reuse = tau * (tau - 2.f) + 2.f * expVal / (sigT * sigT2); 
            flux_integral = (q0 * tau + (sigT * state_flux[g] - q0) * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal) / (3.f * sigT2 * sigT2);

            // Prepare tally
            tally = weight * flux_integral;

            // Atomic operation for flux update
            Kokkos::atomic_add(&FSR_flux[g], tally);

            // Update state flux
            t1 = q0 * expVal / sigT;  
            t2 = q1 * mu * (tau - expVal) / sigT2; 
            t3 = q2 * mu2 * reuse;
            t4 = state_flux[g] * (1.f - expVal);
            state_flux[g] = t1 + t2 + t3 + t4;
        }
    });
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