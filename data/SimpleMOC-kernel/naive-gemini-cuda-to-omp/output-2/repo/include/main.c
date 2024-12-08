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
        Source * sources_d;

        //Allocate and copy data to device
        #pragma omp target enter data map(alloc: SA_d[0:1], sources_h[0:I.source_3D_regions])
	sources_d = initialize_device_sources( I, &SA_h, &SA_d, sources_h); 
	#pragma omp target update from(SA_d)
        #pragma omp target update from(sources_d[0:I.source_3D_regions])

	
	// Build Exponential Table
	Table * table_d = NULL;
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	Table table = buildExponentialTable();
        #pragma omp target enter data map(alloc: table_d)
	#pragma omp target update to(table_d)
	#endif

	// Setup OpenMP threads
	omp_set_num_threads(I.nthreads);

	// Setup OpenMP RNG on Device.  curand is not directly supported in OpenMP offload, 
        // a replacement random number generator will be needed.  
	printf("Setting up OpenMP RNG...\n");
        //curandState * RNG_states; This is removed as curand is not supported
	//#pragma omp target enter data map(alloc: RNG_states[0:I.streams])
	//setup_kernel<<<I.streams/100 + 1, 100>>>(RNG_states, I); This is removed as curand is not supported
	//CudaCheckError(); This is removed as curand is not supported
	//cudaDeviceSynchronize(); This is removed as curand is not supported

	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
        #pragma omp target enter data map(alloc: flux_states[0:N_flux_states*I.egroups])
	//init_flux_states<<< blocks, I.egroups >>> ( flux_states, N_flux_states, I, RNG_states ); This is removed as curand is not supported


	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();
	//cudaDeviceSynchronize(); This is removed as curand is not supported
	printf("Attentuating fluxes across segments...\n");

	// OpenMP timer variables
	double start_time, end_time;

	// Setup kernel call block parameters
	assert( I.segments % I.seg_per_thread == 0 );
	int n_blocks = sqrt(I.segments / I.seg_per_thread);
        dim3 blocks_k(n_blocks, n_blocks);
        if( blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread )
		blocks_k.x++;
	if( blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread )
		blocks_k.y++;
	assert( blocks_k.x * blocks_k.y >= I.segments / I.seg_per_thread );

	// Run Simulation Kernel Loop
	start_time = omp_get_wtime();
        #pragma omp target teams distribute parallel for
	run_kernel(I, sources_d, SA_d, table_d, NULL, flux_states, N_flux_states); //Note:  RNG_states removed as it's not supported in OpenMP
	end_time = omp_get_wtime();


	float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target update from(flux_states[0:N_flux_states * I.egroups])
	//CUDA_CALL( cudaMemcpy( host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float), cudaMemcpyDeviceToHost)); This is removed and replaced with OpenMP update

	printf("Simulation Complete.\n");

	border_print();
	center_print("RESULTS SUMMARY", 79);
	border_print();

	double time = end_time - start_time;
	double tpi = ((double) (time) /
			(double)I.segments / (double) I.egroups) * 1.0e9;
	printf("%-25s%.3f seconds\n", "Runtime:", time);
	printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
	border_print();

        #pragma omp target exit data map(delete: SA_d[0:1], sources_d[0:I.source_3D_regions], table_d, flux_states[0:N_flux_states*I.egroups])
        free(host_flux_states);

	return 0;
}