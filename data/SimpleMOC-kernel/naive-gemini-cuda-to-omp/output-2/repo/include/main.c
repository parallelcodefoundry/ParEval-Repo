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

	// Build Exponential Table
	Table * table_d = NULL;
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	Table table = buildExponentialTable();
        #pragma omp target enter data map(alloc: table[0:1])
        #pragma omp target update to(table[0:1])
	#endif

	// Setup OpenMP offloading environment
        #pragma omp parallel
        {
            #pragma omp target data map(alloc: SA_d.fine_flux_arr[0:I.source_3D_regions*I.fine_axial_intervals*I.egroups], SA_d.fine_source_arr[0:I.source_3D_regions*I.fine_axial_intervals*I.egroups], SA_d.sigT_arr[0:I.source_3D_regions*I.egroups], sources_d[0:I.source_3D_regions])
            {
                #pragma omp target enter data map(to: SA_h[0:1])
                sources_d = initialize_device_sources( I, &SA_h, &SA_d, sources_h );
                #pragma omp target update from(SA_d[0:1])
                #pragma omp target exit data map(release: SA_h[0:1])
                #pragma omp target exit data map(from: SA_d[0:1])
                
		// Setup OpenMP RNG on Device - needs to be reworked for OpenMP offload
		printf("Setting up OpenMP RNG...\n");
		//curandState * RNG_states;
		//CUDA_CALL( cudaMalloc((void **)&RNG_states, I.streams * sizeof(curandState)) );
		//setup_kernel<<<I.streams/100 + 1, 100>>>(RNG_states, I);
		//CudaCheckError();
		//cudaDeviceSynchronize();

		// Allocate Some Flux State vectors to randomly pick from
		printf("Setting up Flux State Vectors...\n");
		float * flux_states;
		int N_flux_states = 10000;
		assert( I.segments >= N_flux_states );
                #pragma omp target enter data map(alloc: flux_states[0:N_flux_states*I.egroups])
		init_flux_states<<< blocks, I.egroups >>> ( flux_states, N_flux_states, I, RNG_states ); // needs to be reworked for OpenMP offload
		printf("Initialization Complete.\n");
            }
        }

	border_print();
	center_print("SIMULATION", 79);
	border_print();
	//cudaDeviceSynchronize(); //Not needed for OpenMP
	printf("Attentuating fluxes across segments...\n");

	// OpenMP timer variables
	double start_time, end_time;
	start_time = omp_get_wtime();
	
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
	#pragma omp target teams distribute parallel for
	for (int i = 0; i < I.segments/I.seg_per_thread; ++i) {
            run_kernel<<<1, I.egroups, I.seg_per_thread * 3 * sizeof(int)>>>(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states); //needs to be reworked for OpenMP offload
	}

	end_time = omp_get_wtime();
	double time = end_time - start_time;

	float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target update from(flux_states[0:N_flux_states*I.egroups])
	//CUDA_CALL( cudaMemcpy( host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float), cudaMemcpyDeviceToHost)); //needs to be reworked for OpenMP offload

	printf("Simulation Complete.\n");

	border_print();
	center_print("RESULTS SUMMARY", 79);
	border_print();

	double tpi = ((double) (time) /
			(double)I.segments / (double) I.egroups) * 1.0e9;
	printf("%-25s%.3f seconds\n", "Runtime:", time);
	printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
	border_print();

	return 0;
}