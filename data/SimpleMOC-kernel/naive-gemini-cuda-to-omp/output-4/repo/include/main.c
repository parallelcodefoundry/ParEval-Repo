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
	//cudaDeviceSynchronize(); //removed as no longer needed

	// Build Exponential Table
	Table * table_d = NULL;
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	Table table = buildExponentialTable();
        #pragma omp target enter data map(alloc:table[0:1])
	#pragma omp target enter data map(to:table[0:1])
	//CUDA_CALL( cudaMalloc((void **) &table_d, sizeof(Table)) ); //removed as no longer needed
	//CUDA_CALL( cudaMemcpy(table_d, &table, sizeof(Table), cudaMemcpyHostToDevice) ); //removed as no longer needed
        table_d = &table;
	#endif

	// Setup OpenMP offloading for blocks / threads.  This needs to be determined dynamically based on the hardware.
        int n_blocks = sqrt(I.segments);
        dim3 blocks(n_blocks, n_blocks);
        if( blocks.x * blocks.y < I.segments )
                blocks.x++;
        if( blocks.x * blocks.y < I.segments )
                blocks.y++;
        assert( blocks.x * blocks.y >= I.segments );

	// Setup OpenMP RNG on Device -  This needs to be re-implemented for OpenMP
	printf("Setting up OpenMP RNG...\n");
	curandState * RNG_states;
        //CUDA_CALL( cudaMalloc((void **)&RNG_states, I.streams * sizeof(curandState)) ); //removed as no longer needed
        RNG_states = (curandState*) malloc(I.streams * sizeof(curandState));
        #pragma omp target enter data map(alloc:RNG_states[0:I.streams])
	#pragma omp target teams distribute parallel for
        for (int i = 0; i < I.streams; i++) {
            curand_init(1234, i, 0, &RNG_states[i]);
        }
	//setup_kernel<<<I.streams/100 + 1, 100>>>(RNG_states, I); //removed as no longer needed
	//CudaCheckError(); //removed as no longer needed
	//cudaDeviceSynchronize(); //removed as no longer needed

	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
        //CUDA_CALL( cudaMalloc((void **) &flux_states, N_flux_states * I.egroups * sizeof(float)) ); //removed as no longer needed
        flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target enter data map(alloc:flux_states[0:N_flux_states*I.egroups])
	init_flux_states<<< blocks, I.egroups >>> ( flux_states, N_flux_states, I, RNG_states ); //replace with OpenMP version


	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();
	//cudaDeviceSynchronize(); //removed as no longer needed
	printf("Attentuating fluxes across segments...\n");

	// OpenMP timer variables
	double start_time, end_time;
	start_time = omp_get_wtime();
	
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
	#pragma omp target teams distribute parallel for
	for (int i = 0; i < I.segments / I.seg_per_thread; i++) {
            run_kernel<<<1, I.egroups, I.seg_per_thread * 3 *sizeof(int)>>>(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states);
        }
        //CudaCheckError(); //removed as no longer needed
	//cudaEventRecord(stop, 0); //removed as no longer needed
	//cudaEventSynchronize(start); //removed as no longer needed
	//cudaEventSynchronize(stop); //removed as no longer needed
	//cudaEventElapsedTime(&time, start, stop); //removed as no longer needed
	//cudaDeviceSynchronize(); //removed as no longer needed
	end_time = omp_get_wtime();
	double time = end_time - start_time;

	float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target exit data map(from:flux_states[0:N_flux_states*I.egroups])
        #pragma omp target update from(flux_states[0:N_flux_states*I.egroups])
	//CUDA_CALL( cudaMemcpy( host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float), cudaMemcpyDeviceToHost)); //removed as no longer needed
        memcpy(host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float));

	printf("Simulation Complete.\n");

	border_print();
	center_print("RESULTS SUMMARY", 79);
	border_print();

	double tpi = ((double) (time) /
			(double)I.segments / (double) I.egroups) * 1.0e9;
	printf("%-25s%.3f seconds\n", "Runtime:", time);
	printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
	border_print();

        #pragma omp target exit data map(release:RNG_states[0:I.streams])
        #pragma omp target exit data map(release:flux_states[0:N_flux_states*I.egroups])
        #pragma omp target exit data map(release:table_d[0:1])
	free(RNG_states);
        free(flux_states);
	free(host_flux_states);
	free(sources_h);
	free(SA_h.fine_source_arr);
	free(SA_h.fine_flux_arr);
	free(SA_h.sigT_arr);

	return 0;
}