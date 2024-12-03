#include "SimpleMOC-kernel_header.h"

/* My parallelization scheme here is to basically have a single
 * block be a geometrical segment, with each thread within the
 * block represent a single energy phase. On the CPU, the
 * inner SIMD-ized loop is over energy (i.e, 100 energy groups).
 * This should allow for each BLOCK to have:
 * 		- A single state variable for the RNG
 * 		- A set of __shared__ SIMD vectors, each thread id being its idx
 */

#pragma omp target teams distribute parallel for simd private(localState, blockId, g, q0, q1, q2, sigT, tau, sigT2, expVal, reuse, flux_integral, tally, t1, t2, t3, t4)
for (int blockId_outer = 0; blockId_outer < I.segments / I.seg_per_thread; ++blockId_outer){
    int blockId = blockId_outer; // geometric segment	

    // Assign RNG state.  Note:  OpenMP's random number generation needs to be addressed. This is a placeholder.
    curandState localState; // Requires proper initialization based on OpenMP thread ID
    //curand_init(1234, omp_get_thread_num(), 0, &localState); //Example - needs proper seeding strategy

    blockId *= I.seg_per_thread;
    blockId--;

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

    // Randomized variables (common accross all thread within block)
    //Shared memory needs to be replaced with a different synchronization mechanism in OpenMP.
    int state_flux_id[I.seg_per_thread];
    int QSR_id[I.seg_per_thread];
    int FAI_id[I.seg_per_thread];

    #pragma omp simd
    for( int i = 0; i < I.seg_per_thread; i++ )
    {
        // Requires proper random number generation for OpenMP
        state_flux_id[i] = rand() % N_state_fluxes; 
        QSR_id[i] = rand() % I.source_3D_regions;
        FAI_id[i] = rand() % I.fine_axial_intervals;
    }


    for( int i = 0; i < I.seg_per_thread; i++ )
    {
        blockId++;

        float *  state_flux = &state_fluxes[state_flux_id[i]];

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
        float *  FSR_flux = &SA.fine_flux_arr[ S[QSR_id[i]].fine_flux_id + FAI_id[i] * egroups];

        //The following conditional logic needs to handle potential race conditions with OpenMP's threading model.  Atomic operations or critical sections might be necessary.

        if( FAI_id[i] == 0 )
        {
            float *   f2 = &SA.fine_source_arr[ S[QSR_id[i]].fine_source_id + (FAI_id[i])*egroups];
            float *   f3 = &SA.fine_source_arr[ S[QSR_id[i]].fine_source_id + (FAI_id[i]+1)*egroups];
            float y2 = f2[g];
            float y3 = f3[g];
            float c0 = y2;
            float c1 = (y3 - y2) / dz;
            q0 = c0 + c1*zin;
            q1 = c1;
            q2 = 0;
        }
        else if ( FAI_id[i] == I.fine_axial_intervals - 1 )
        {
            //Similar to the above, needs to handle potential race conditions.
        }
        else
        {
            //Similar to the above, needs to handle potential race conditions.
        }

        sigT = SA.sigT_arr[ S[QSR_id[i]].sigT_id + g];

        tau = sigT * ds;
        sigT2 = sigT * sigT;

        #ifdef TABLE
        interpolateTable( table, tau, &expVal );  
        #else
        expVal = 1.f - expf( -tau); // EXP function is fater than table lookup
        #endif

        reuse = tau * (tau - 2.f) + 2.f * expVal
            / (sigT * sigT2); 

        flux_integral = (q0 * tau + (sigT * state_flux[g] - q0)
                * expVal) / sigT2 + q1 * mu * reuse + q2 * mu2 
            * (tau * (tau * (tau - 3.f) + 6.f) - 6.f * expVal) 
            / (3.f * sigT2 * sigT2);

        tally = weight * flux_integral;

        #pragma omp atomic update
        FSR_flux[g] += tally;


        t1 = q0 * expVal / sigT;  
        t2 = q1 * mu * (tau - expVal) / sigT2; 
        t3 =	q2 * mu2 * reuse;
        t4 = state_flux[g] * (1.f - expVal);
        state_flux[g] = t1 + t2 + t3 + t4;

    }
}
}
/* Interpolates a formed exponential table to compute ( 1- exp(-x) )
 *  at the desired x value */
__device__ void interpolateTable(Table *  table, float x, float *  out)
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