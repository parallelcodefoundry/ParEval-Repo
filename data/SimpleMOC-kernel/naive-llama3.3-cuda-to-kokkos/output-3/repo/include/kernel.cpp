#include "SimpleMOC-kernel_header.hpp"

// My parallelization scheme here is to basically have a single
// block be a geometrical segment, with each thread within the
// block represent a single energy phase. On the CPU, the
// inner SIMD-ized loop is over energy (i.e, 100 energy groups).
// This should allow for each BLOCK to have:
// 		- A single state variable for the RNG
// 		- A set of __shared__ SIMD vectors, each thread id being its idx
void run_kernel(const Input& I, const Source* S,
                const Source_Arrays& SA, const Table* table, curandState* state,
                float* state_fluxes, int N_state_fluxes) {
  Kokkos::parallel_for(
      Kokkos::RangePolicy<>(0, I.segments / I.seg_per_thread),
      KOKKOS_LAMBDA(const int blockId) {
        // Assign RNG state
        curandState* localState = &state[blockId % I.streams];

        int g = Kokkos::ThreadVectorRange(0, I.egroups).vector_length();
        int threadId = Kokkos::ThreadVectorRange(0, I.egroups).vector_rank();

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
        Kokkos::View<int*, Kokkos::MemoryUnmanaged> state_flux_id("state_flux_id", I.seg_per_thread);
        Kokkos::View<int*, Kokkos::MemoryUnmanaged> QSR_id("QSR_id", I.seg_per_thread);
        Kokkos::View<int*, Kokkos::MemoryUnmanaged> FAI_id("FAI_id", I.seg_per_thread);

        if (threadId == 0) {
          for (int i = 0; i < I.seg_per_thread; i++) {
            state_flux_id(i) = curand(localState) % N_state_fluxes;
            QSR_id(i) = curand(localState) % I.source_3D_regions;
            FAI_id(i) = curand(localState) % I.fine_axial_intervals;
          }
        }

        Kokkos::fence();

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
          float* FSR_flux = &SA.fine_flux_arr[S[QSR_id(i)].fine_flux_id + FAI_id(i) * egroups];

          if (FAI_id(i) == 0) {
            float* f2 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i)) * egroups];
            float* f3 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i) + 1) * egroups];
            // cycle over energy groups
            // load neighboring sources
            float y2 = f2[threadId];
            float y3 = f3[threadId];

            // do linear "fitting"
            float c0 = y2;
            float c1 = (y3 - y2) / dz;

            // calculate q0, q1, q2
            q0 = c0 + c1 * zin;
            q1 = c1;
            q2 = 0;
          } else if (FAI_id(i) == I.fine_axial_intervals - 1) {
            float* f1 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i) - 1) * egroups];
            float* f2 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i)) * egroups];
            // cycle over energy groups
            // load neighboring sources
            float y1 = f1[threadId];
            float y2 = f2[threadId];

            // do linear "fitting"
            float c0 = y2;
            float c1 = (y2 - y1) / dz;

            // calculate q0, q1, q2
            q0 = c0 + c1 * zin;
            q1 = c1;
            q2 = 0;
          } else {
            float* f1 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i) - 1) * egroups];
            float* f2 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i)) * egroups];
            float* f3 = &SA.fine_source_arr[S[QSR_id(i)].fine_source_id + (FAI_id(i) + 1) * egroups];
            // cycle over energy groups
            // load neighboring sources
            float y1 = f1[threadId];
            float y2 = f2[threadId];
            float y3 = f3[threadId];

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
          sigT = SA.sigT_arr[S[QSR_id(i)].sigT_id + threadId];

          // calculate common values for efficiency
          tau = sigT * ds;
          sigT2 = sigT * sigT;

#ifdef TABLE
          interpolateTable(table, tau, &expVal);
#else
          expVal = 1.f - expf(-tau);  // EXP function is fater than table lookup
#endif

          // Flux Integral

          // Re-used Term
          reuse = tau * (tau - 2.f) + 2.f * expVal / (sigT * sigT2);

          // add contribution to new source flux
          flux_integral = (q0 * tau + (sigT * state_fluxes[state_flux_id(i) + threadId] - q0) * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal) / (3.f * sigT2 * sigT2);

          // Prepare tally
          tally = weight * flux_integral;

          // SHOULD BE ATOMIC HERE!
          // FSR_flux[threadId] += tally;
          Kokkos::atomic_add(&FSR_flux[threadId], (float)tally);

          // Term 1
          t1 = q0 * expVal / sigT;
          // Term 2
          t2 = q1 * mu * (tau - expVal) / sigT2;
          // Term 3
          t3 = q2 * mu2 * reuse;
          // Term 4
          t4 = state_fluxes[state_flux_id(i) + threadId] * (1.f - expVal);
          // Total psi
          state_fluxes[state_flux_id(i) + threadId] = t1 + t2 + t3 + t4;
        }
      });
}

// Interpolates a formed exponential table to compute ( 1- exp(-x) )
//  at the desired x value
void interpolateTable(const Table* table, float x, float* out) {
  // check to ensure value is in domain
  if (x > table->maxVal) {
    *out = 1.0f;
  } else {
    int interval = (int)(x / table->dx + 0.5f * table->dx);
    interval = interval * 2;
    float slope = table->values[interval];
    float intercept = table->values[interval + 1];
    float val = slope * x + intercept;
    *out = val;
  }
}