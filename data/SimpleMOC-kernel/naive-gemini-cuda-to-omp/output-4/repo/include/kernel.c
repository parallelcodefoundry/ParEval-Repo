#include "SimpleMOC-kernel_header.h"

/* My parallelization scheme here is to basically have a single
 * block be a geometrical segment, with each thread within the
 * block represent a single energy phase. On the CPU, the
 * inner SIMD-ized loop is over energy (i.e, 100 energy groups).
 * This should allow for each BLOCK to have:
 * 		- A single state variable for the RNG
 * 		- A set of __shared__ SIMD vectors, each thread id being its idx
 */

void run_kernel( Input I, Source *  S,
		Source_Arrays SA, Table *  table, unsigned long long * state, // Changed curandState to unsigned long long
		float *  state_fluxes, int N_state_fluxes)
{
	#pragma omp parallel for
	for (long blockId = 0; blockId < I.segments / I.seg_per_thread; blockId++) {
		// Assign RNG state.  We'll need a different RNG approach for OpenMP.
		unsigned long long * localState = &state[blockId % I.streams]; //  Adjust for OpenMP's RNG

		long segmentIndex = blockId * I.seg_per_thread;


		#pragma omp simd
		for (int g = 0; g < I.egroups; g++) { //Each energy group (g) is one thread in a block

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

			//We need to manage shared memory differently in OpenMP.  Consider using arrays instead.
			int state_flux_id[I.seg_per_thread];
			int QSR_id[I.seg_per_thread];
			int FAI_id[I.seg_per_thread];

			// Initialize random numbers for each segment - replace with proper OpenMP RNG
			for( int i = 0; i < I.seg_per_thread; i++ ) {
				state_flux_id[i] = (int) (erand48(seed) * N_state_fluxes); //Replace with OpenMP RNG
				QSR_id[i] = (int) (erand48(seed) * I.source_3D_regions); //Replace with OpenMP RNG
				FAI_id[i] = (int) (erand48(seed) * I.fine_axial_intervals); //Replace with OpenMP RNG
			}

			for (int i = 0; i < I.seg_per_thread; i++) {
				segmentIndex++;
				float * state_flux = &state_fluxes[state_flux_id[i]];

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

				// load fine source region flux vector
				float * FSR_flux = &SA.fine_flux_arr[S[QSR_id[i]].fine_flux_id + FAI_id[i] * I.egroups];


				// The following code block remains largely the same, just remove CUDA intrinsics
				if (FAI_id[i] == 0) {
					// ... (Existing code for FAI_id == 0) ...
				} else if (FAI_id[i] == I.fine_axial_intervals - 1) {
					// ... (Existing code for FAI_id == I.fine_axial_intervals - 1) ...
				} else {
					// ... (Existing code for other FAI_id values) ...
				}


				// load total cross section
				sigT = SA.sigT_arr[S[QSR_id[i]].sigT_id + g];


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
				flux_integral = (q0 * tau + (sigT * state_flux[g] - q0) * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal) / (3.f * sigT2 * sigT2);

				// Prepare tally
				tally = weight * flux_integral;

				// Atomic add - needs OpenMP equivalent
				#pragma omp atomic
					FSR_flux[g] += tally;


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
		}
	}
}

/* Interpolates a formed exponential table to compute ( 1- exp(-x) )
 *  at the desired x value */
void interpolateTable(Table * table, float x, float * out)
{
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