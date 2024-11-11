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
	
	// Allocate & Copy Fine Source Data
	long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
	SA_d.fine_source_arr = (float *) malloc( N_fine * sizeof(float));
	memcpy(SA_d.fine_source_arr, SA_h.fine_source_arr, N_fine * sizeof(float));

	// Allocate & Copy Fine Flux Data
	SA_d.fine_flux_arr = (float *) malloc( N_fine * sizeof(float));
	memcpy(SA_d.fine_flux_arr, SA_h.fine_flux_arr, N_fine * sizeof(float));

	// Allocate & Copy SigT Data
	long N_sigT = I.source_3D_regions * I.egroups;
	SA_d.sigT_arr = (float *) malloc( N_sigT * sizeof(float));
	memcpy(SA_d.sigT_arr, SA_h.sigT_arr, N_sigT * sizeof(float));

	// Allocate & Copy Source Array Data
	Source * sources_d = (Source *) malloc( I.source_3D_regions * sizeof(Source));
	memcpy(sources_d, sources_h, I.source_3D_regions * sizeof(Source));

	#pragma omp target data map(to: SA_d.fine_source_arr[0:N_fine], SA_d.fine_flux_arr[0:N_fine], SA_d.sigT_arr[0:N_sigT], sources_d[0:I.source_3D_regions])
	{
		// Build Exponential Table
		Table * table_d = NULL;
		#ifdef TABLE
		printf("Building Exponential Table...\n");
		Table table = buildExponentialTable();
		table_d = (Table *) malloc( sizeof(Table));
		memcpy(table_d, &table, sizeof(Table));
		#endif

		// Setup CUDA blocks / threads
		int n_blocks = sqrt(I.segments);
		dim3 blocks(n_blocks, n_blocks);
		if( blocks.x * blocks.y < I.segments )
			blocks.x++;
		if( blocks.x * blocks.y < I.segments )
			blocks.y++;
		assert( blocks.x * blocks.y >= I.segments );

		// Setup CUDA RNG on Device
		printf("Setting up CUDA RNG...\n");
		curandState * RNG_states = (curandState *) malloc( I.streams * sizeof(curandState));
		#pragma omp target teams distribute parallel for
		for (int i = 0; i < I.streams; i++) {
			curand_init(1234, i, 0, &RNG_states[i]);
		}

		// Allocate Some Flux State vectors to randomly pick from
		printf("Setting up Flux State Vectors...\n");
		float * flux_states = (float *) malloc( N_flux_states * I.egroups * sizeof(float));
		int N_flux_states = 10000;
		assert( I.segments >= N_flux_states );
		#pragma omp target teams distribute parallel for
		for (int i = 0; i < N_flux_states * I.egroups; i++) {
			flux_states[i] = curand_uniform(&RNG_states[i % I.streams]);
		}

		printf("Initialization Complete.\n");
		border_print();
		center_print("SIMULATION", 79);
		border_print();

		printf("Attentuating fluxes across segments...\n");

		// CUDA timer variables
		cudaEvent_t start, stop;
		cudaEventCreate(&start);
		cudaEventCreate(&stop);
		float time = 0;
		
		// Setup kernel call block parameters
		assert( I.segments % I.seg_per_thread == 0 );
		n_blocks = sqrt(I.segments / I.seg_per_thread);
		dim3 blocks_k(n_blocks, n_blocks);
		if( blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread )
			blocks_k.x++;
		if( blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread )
			blocks_k.y++;
		assert( blocks_k.x * blocks_k.y >= I.segments / I.seg_per_thread );

		// Run Simulation Kernel Loop
		cudaEventRecord(start, 0);

		#pragma omp target teams distribute parallel for collapse(2)
		for (int blockId = 0; blockId < I.segments / I.seg_per_thread; blockId++) {
			for (int g = 0; g < I.egroups; g++) {
				run_kernel_omp(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states, blockId, g);
			}
		}
		CudaCheckError();
		cudaEventRecord(stop, 0);
		cudaEventSynchronize(start);
		cudaEventSynchronize(stop);
		cudaEventElapsedTime(&time, start, stop);
		cudaDeviceSynchronize();

		float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
		memcpy( host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float));

		printf("Simulation Complete.\n");

		border_print();
		center_print("RESULTS SUMMARY", 79);
		border_print();

		double tpi = ((double) (time/1000.0) /
				(double)I.segments / (double) I.egroups) * 1.0e9;
		printf("%-25s%.3f seconds\n", "Runtime:", time / 1000.0);
		printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
		border_print();

		free(flux_states);
		free(RNG_states);
		#ifdef TABLE
		free(table_d);
		#endif
	}
	free(sources_d);
	free(SA_d.fine_source_arr);
	free(SA_d.fine_flux_arr);
	free(SA_d.sigT_arr);
	free(sources_h);
	free(SA_h.fine_source_arr);
	free(SA_h.fine_flux_arr);
	free(SA_h.sigT_arr);

	return 0;
}