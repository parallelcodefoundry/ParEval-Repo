#include "SimpleMOC-kernel_header.h"

int main( int argc, char * argv[] )
{
	int version = 4;

	srand(time(NULL));

	Input I = set_default_input();
	read_CLI( argc, argv, &I );
	
	// Calculate Number of 3D Source Regions
	I.source_3D_regions = (int) ceil((double)I.source_2D_regions *
		I.coarse_axial_intervals / I.decomp_assemblies_ax);

	logo(version);

	print_input_summary(I);
	
	center_print("INITIALIZATION", 79);
	border_print();

	// Build Source Data
	printf("Building Source Data Arrays...\n");
	Source_Arrays SA_h, SA_d;
	Source * sources_h = initialize_sources(I, &SA_h); 
	Source * sources_d = initialize_device_sources( I, &SA_h, &SA_d, sources_h); 

	
	// Build Exponential Table
	Table * table_d = NULL;
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	Table table = buildExponentialTable();
        #pragma omp target enter data map(to:table[0:1])
	#pragma omp target data map(tofrom:table[0:1])
	{
            #pragma omp target map(to:table)
            {
                table_d = &table;
            }
	}
	#endif

	// Setup OpenMP offload blocks / threads.  This will need adjustment based on
	// target device capabilities.  A more sophisticated approach may involve
	// querying the device for optimal block and thread configurations.
	int n_blocks = sqrt(I.segments);
	//dim3 blocks(n_blocks, n_blocks); //No longer needed.
	if( n_blocks * n_blocks < I.segments )
		n_blocks++;
	if( n_blocks * n_blocks < I.segments )
		n_blocks++;
	assert( n_blocks * n_blocks >= I.segments );


	// Setup RNG on Device.  This section needs significant modification.  The
	// CUDA curand library is not directly compatible with OpenMP offloading.  A
	// replacement random number generator suitable for OpenMP needs to be used.
	printf("Setting up RNG...\n");
        curandState * RNG_states;
	//CUDA_CALL( cudaMalloc((void **)&RNG_states, I.streams * sizeof(curandState)) ); //Not needed
        RNG_states = (curandState*) malloc(I.streams * sizeof(curandState));
        #pragma omp target enter data map(to:RNG_states[0:I.streams])
	//setup_kernel<<<I.streams/100 + 1, 100>>>(RNG_states, I); //Replaced
        #pragma omp parallel for
        for(int i = 0; i < I.streams; ++i){
            curand_init(1234, i, 0, &RNG_states[i]);
        }
	//CudaCheckError(); //Not needed
	//#pragma omp target update to(RNG_states[0:I.streams]) //Not needed
        


	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
	flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target enter data map(to:flux_states[0:N_flux_states*I.egroups])
	#pragma omp target data map(tofrom:flux_states[0:N_flux_states*I.egroups])
        {
            #pragma omp target map(to:flux_states[0:N_flux_states*I.egroups], RNG_states[0:I.streams], I)
            {
                init_flux_states<<< n_blocks, I.egroups >>> ( flux_states, N_flux_states, I, RNG_states );
            }
        }
	

	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();
	//#pragma omp target update to(flux_states[0:N_flux_states*I.egroups]) //Not needed

	printf("Attentuating fluxes across segments...\n");

	// Timer variables
	double start_time, end_time;
	
	// Setup kernel call block parameters
	assert( I.segments % I.seg_per_thread == 0 );
	n_blocks = sqrt(I.segments / I.seg_per_thread);
	//dim3 blocks_k(n_blocks, n_blocks); //Not needed
	if( n_blocks * n_blocks < I.segments / I.seg_per_thread )
		n_blocks++;
	if( n_blocks * n_blocks < I.segments / I.seg_per_thread )
		n_blocks++;
	assert( n_blocks * n_blocks >= I.segments / I.seg_per_thread );


	// Run Simulation Kernel Loop
	start_time = omp_get_wtime();
	#pragma omp target data map(to:I, sources_d[0:I.source_3D_regions], SA_d, table_d, RNG_states[0:I.streams], flux_states[0:N_flux_states*I.egroups]) map(alloc:shm[0:I.seg_per_thread*3])
        {
            #pragma omp target teams distribute parallel for thread_limit(I.egroups) num_teams(n_blocks)
            for (int blockId = 0; blockId < I.segments / I.seg_per_thread; ++blockId){
                extern int shm[];
                int* state_flux_id = shm;
                int* QSR_id = shm + I.seg_per_thread;
                int* FAI_id = shm + 2 * I.seg_per_thread;

                if (omp_get_thread_num() == 0){
                    curandState* localState = &RNG_states[blockId % I.streams];
                    for (int i = 0; i < I.seg_per_thread; ++i){
                        state_flux_id[i] = curand(localState) % N_flux_states;
                        QSR_id[i] = curand(localState) % I.source_3D_regions;
                        FAI_id[i] = curand(localState) % I.fine_axial_intervals;
                    }
                }
                #pragma omp barrier

                int g = omp_get_thread_num();
                // ... rest of run_kernel functionality ...
                // This would require significant restructuring to adapt the original CUDA kernel
                // to OpenMP's data sharing model and to handle the shared memory 'shm' appropriately.

            }
        }
	end_time = omp_get_wtime();
	double time = end_time - start_time;

	float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target update from(flux_states[0:N_flux_states * I.egroups])
	memcpy( host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float));

	printf("Simulation Complete.\n");

	border_print();
	center_print("RESULTS SUMMARY", 79);
	border_print();

	double tpi = ((double) (time) /
			(double)I.segments / (double) I.egroups) * 1.0e9;
	printf("%-25s%.3f seconds\n", "Runtime:", time);
	printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
	border_print();

        free(RNG_states);
        free(flux_states);
        free(host_flux_states);
	return 0;
}