#include "SimpleMOC-kernel_header.hpp"

// Kokkos kernel for attenuating fluxes across segments
KOKKOS_KERNEL void run_kernel(const Input& I, const Source* S,
                             const Source_Arrays& SA, const Table* table,
                             const Kokkos::View<curandState*, Kokkos::HostSpace>& state,
                             Kokkos::View<float*, Kokkos::DeviceSpace>& state_fluxes,
                             int N_state_fluxes) {
    int blockId = kokkos::team_size * kokkos::team_rank + kokkos::team_thread_rank; // geometric segment

    if (blockId >= I.segments / I.seg_per_thread) return;

    // Assign RNG state.  Kokkos::View already handles memory allocation and management
    const curandState& localState = state(blockId % I.streams);

    blockId *= I.seg_per_thread;
    blockId--;

    int g = kokkos::team_thread_rank; // Each energy group (g) is one thread in a block


    // Thread Local (i.e., specific to E group) variables
    float q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4;

    // Shared memory for randomized variables (common across all threads within a team)
    Kokkos::TeamThreadSpace teamSpace = kokkos::thread_space( kokkos::get_team() );
    Kokkos::View<int*, Kokkos::TeamThreadSpace> state_flux_id("state_flux_id", teamSpace, I.seg_per_thread);
    Kokkos::View<int*, Kokkos::TeamThreadSpace> QSR_id("QSR_id", teamSpace, I.seg_per_thread);
    Kokkos::View<int*, Kokkos::TeamThreadSpace> FAI_id("FAI_id", teamSpace, I.seg_per_thread);

    if (kokkos::team_thread_rank == 0) {
        for (int i = 0; i < I.seg_per_thread; i++) {
            state_flux_id(i) = curand(&localState) % N_state_fluxes;
            QSR_id(i) = curand(&localState) % I.source_3D_regions;
            FAI_id(i) = curand(&localState) % I.fine_axial_intervals;
        }
    }
    Kokkos::single( kokkos::get_team(), [&](){
        kokkos::get_team().fence();
    });


    for (int i = 0; i < I.seg_per_thread; i++) {
        blockId++;

        float* state_flux = &state_fluxes(state_flux_id(i) * I.egroups);


        // Attenuate Segment
        float dz = 0.1f;
        float zin = 0.3f;
        float weight = 0.5f;
        float mu = 0.9f;
        float mu2 = 0.3f;
        float ds = 0.7f;
        const int egroups = I.egroups;

        // load fine source region flux vector
        float* FSR_flux = &SA.fine_flux_arr(S(QSR_id(i)).fine_flux_id + FAI_id(i) * egroups);

        // Load neighboring sources and calculate q0, q1, q2 based on FAI_id(i) similar to CUDA version
        if( FAI_id(i) == 0 ) {
            // ... (Code for FAI_id(i) == 0 remains the same)
        } else if ( FAI_id(i) == I.fine_axial_intervals - 1 ) {
            // ... (Code for FAI_id(i) == I.fine_axial_intervals - 1 remains the same)
        } else {
            // ... (Code for the else condition remains the same)
        }

        sigT = SA.sigT_arr(S(QSR_id(i)).sigT_id + g);
        tau = sigT * ds;
        sigT2 = sigT * sigT;

        //Interpolate or calculate expVal
        if constexpr (defined(TABLE)) {
            interpolateTable(table, tau, &expVal);
        } else {
            expVal = 1.f - expf(-tau);
        }


        reuse = tau * (tau - 2.f) + 2.f * expVal / (sigT * sigT2);
        flux_integral = (q0 * tau + (sigT * state_flux[g] - q0) * expVal) / sigT2 + q1 * mu * reuse +
                        q2 * mu2 * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal) / (3.f * sigT2 * sigT2);
        tally = weight * flux_integral;

        Kokkos::atomic_add(&FSR_flux[g], tally); // Atomic add

        // Calculations for t1, t2, t3, t4 remain the same

        state_flux[g] = t1 + t2 + t3 + t4;
    }
}

// Kokkos device function for table interpolation
KOKKOS_FUNCTION void interpolateTable(const Table* table, float x, float* out) {
  // ... (interpolation code remains the same)
}
