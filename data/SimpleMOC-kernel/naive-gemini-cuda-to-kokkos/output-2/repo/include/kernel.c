#include "SimpleMOC-kernel_header.hpp"

/* My parallelization scheme here is to basically have a single
 * block be a geometrical segment, with each thread within the
 * block represent a single energy phase. On the CPU, the
 * inner SIMD-ized loop is over energy (i.e, 100 energy groups).
 * This should allow for each BLOCK to have:
 * 		- A single state variable for the RNG
 * 		- A set of __shared__ SIMD vectors, each thread id being its idx
 */

KOKKOS_INLINE_FUNCTION
void interpolateTable(const Table &table, float x, float *out) {
    // check to ensure value is in domain
    if (x > table.maxVal)
        *out = 1.0f;
    else {
        int interval = (int)(x / table.dx + 0.5f * table.dx);
        interval = interval * 2;
        float slope = table.values[interval];
        float intercept = table.values[interval + 1];
        float val = slope * x + intercept;
        *out = val;
    }
}


KOKKOS_FUNCTION
void run_kernel_impl(const Input &I, const Source *S,
                     const Source_Arrays &SA, const Table &table,
                     const curandState &state, float *state_fluxes,
                     int N_state_fluxes, int blockId, int g, int i,
                     int* state_flux_id, int* QSR_id, int* FAI_id) {

    float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4;

    int blockId_temp = blockId * I.seg_per_thread + i;
    if (blockId_temp >= I.segments)
      return;

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
    const float *FSR_flux = &SA.fine_flux_arr[S[QSR_id[i]].fine_flux_id + FAI_id[i] * egroups];

    if (FAI_id[i] == 0) {
        const float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
        const float *f3 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] + 1) * egroups];
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
        const float *f1 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] - 1) * egroups];
        const float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
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
        const float *f1 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] - 1) * egroups];
        const float *f2 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i]) * egroups];
        const float *f3 = &SA.fine_source_arr[S[QSR_id[i]].fine_source_id + (FAI_id[i] + 1) * egroups];
        // cycle over energy groups
        // load neighboring sources
        float y1 = f1[g];
        float y2 = f2[g];
        float y3 = f3[g];

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
    sigT = SA.sigT_arr[S[QSR_id[i]].sigT_id + g];

    // calculate common values for efficiency
    tau = sigT * ds;
    sigT2 = sigT * sigT;

#ifdef TABLE
    interpolateTable(table, tau, &expVal);
#else
    expVal = 1.f - expf(-tau); // EXP function is fater than table lookup
#endif

    // Flux Integral

    // Re-used Term
    reuse = tau * (tau - 2.f) + 2.f * expVal / (sigT * sigT2);

    // add contribution to new source flux
    flux_integral = (q0 * tau + (sigT * state_flux[g] - q0) * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal) / (3.f * sigT2 * sigT2);

    // Prepare tally
    tally = weight * flux_integral;

    // SHOULD BE ATOMIC HERE!
    Kokkos::atomic_add(&FSR_flux[g], tally);

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


void run_kernel(const Input &I, const Source *S,
                 const Source_Arrays &SA, const Table &table,
                 const Kokkos::View<curandState*>& state,
                 const Kokkos::View<float*>& state_fluxes,
                 int N_state_fluxes) {

    Kokkos::parallel_for(Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(0, I.segments / I.seg_per_thread),
        KOKKOS_LAMBDA(int blockId) {
            Kokkos::parallel_for(Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(0, I.egroups),
                KOKKOS_LAMBDA(int g) {
                    Kokkos::parallel_for(Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(0, I.seg_per_thread),
                        KOKKOS_LAMBDA(int i) {
                            //Shared memory needs to be handled differently in Kokkos
                            //Allocate temporary arrays for shared memory variables
                            int state_flux_id[I.seg_per_thread];
                            int QSR_id[I.seg_per_thread];
                            int FAI_id[I.seg_per_thread];

                            //Simulate shared memory initialization - needs to be thread safe
                            if (i == 0) {
                                for (int j = 0; j < I.seg_per_thread; ++j) {
                                    state_flux_id[j] = curand(&state(blockId % I.streams)) % N_state_fluxes;
                                    QSR_id[j] = curand(&state(blockId % I.streams)) % I.source_3D_regions;
                                    FAI_id[j] = curand(&state(blockId % I.streams)) % I.fine_axial_intervals;
                                }
                            }

                            // Barrier - Kokkos handles this automatically within parallel_for


                            run_kernel_impl(I, S, SA, table, state(blockId % I.streams), state_fluxes.data(), N_state_fluxes, blockId, g, i, state_flux_id, QSR_id, FAI_id);

                        });
                });
        });
}