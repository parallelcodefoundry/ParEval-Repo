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
		Source_Arrays SA, Table *  table, curandState *  state,
		float *  state_fluxes, int N_state_fluxes)
{
	#pragma omp target teams distribute parallel for simd private(q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4) map(to: I, S[0:I.source_3D_regions], SA.fine_flux_arr[0:I.source_3D_regions*I.fine_axial_intervals*I.egroups], SA.fine_source_arr[0:I.source_3D_regions*I.fine_axial_intervals*I.egroups], SA.sigT_arr[0:I.source_3D_regions*I.egroups], state[0:I.streams], state_fluxes[0:N_state_fluxes*I.egroups]) map(from: SA.fine_flux_arr[0:I.source_3D_regions*I.fine_axial_intervals*I.egroups])
	for (long blockId = 0; blockId < I.segments / I.seg_per_thread; blockId++) {
		int g = omp_get_thread_num(); // Each energy group (g) is one thread in a block

		// Thread Local (i.e., specific to E group) variables
		// Similar to SIMD vectors in CPU code
		float q0           ;
		float q1           ;
		float q2           ;
		float sigT         ;
		float tau          ;
		float sigT2        ;
		float expVal       ;
		float reuse        ;
		float flux_integral;
		float tally        ;
		float t1           ;
		float t2           ;
		float t3           ;
		float t4           ;

		//Assign RNG state.  Need to manage this differently in OpenMP offload.
		curandState *  localState = &state[blockId % I.streams];

		long segmentId = blockId * I.seg_per_thread + g; // Adjust for SIMD

		// Randomized variables (common accross all thread within block)
		//Shared memory is managed differently in OpenMP offload.  Consider using an array instead.
		int state_flux_id = curand(localState) % N_state_fluxes;
		int QSR_id = curand(localState) % I.source_3D_regions;
		int FAI_id = curand(localState) % I.fine_axial_intervals;


		float *  state_flux = &state_fluxes[state_flux_id];

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
		float *  FSR_flux = &SA.fine_flux_arr[ S[QSR_id].fine_flux_id + FAI_id * egroups];

		//Source interpolation logic (similar to CUDA code)
		if( FAI_id == 0 ) {
			//...
		} else if ( FAI_id == I.fine_axial_intervals - 1 ) {
			//...
		} else {
			//...
		}

		// load total cross section
		sigT = SA.sigT_arr[ S[QSR_id].sigT_id + g];

		// calculate common values for efficiency
		tau = sigT * ds;
		sigT2 = sigT * sigT;

		#ifdef TABLE
		interpolateTable( table, tau, &expVal );  
		#else
		expVal = 1.f - expf( -tau); // EXP function is fater than table lookup
		#endif

		// Flux Integral Calculation (similar to CUDA code)
		//...

		// Atomic add is handled differently in OpenMP.  Use `atomic` clause.
		#pragma omp atomic update
		FSR_flux[g] += tally;

		// Update state_flux (similar to CUDA code)
		//...

	}
}	

/* Interpolates a formed exponential table to compute ( 1- exp(-x) )
 *  at the desired x value */
void interpolateTable(Table *  table, float x, float *  out)
{
	// check to ensure value is in domain
	if( x > table->maxVal )
		*out = 1.0f;
	else
	{
		int interval = (int) ( x / table->dx + 0.5f * table->dx );
		interval = interval * 2;
		float slope = table->values[ interval ];
		float intercept = table->values[ interval + 1 ];
		float val = slope * x + intercept;
		*out = val;
	}
}