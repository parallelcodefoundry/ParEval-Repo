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

	//Offload source data initialization to device
	#pragma omp target data map(to: SA_h, sources_h[0:I.source_3D_regions]) map(alloc: SA_d, sources_d[0:I.source_3D_regions])
	{
		#pragma omp target map(tofrom: SA_d, sources_d[0:I.source_3D_regions])
		{
			sources_d = initialize_device_sources( I, &SA_h, &SA_d, sources_h); 
		}
	}


	// Build Exponential Table
	Table * table_d = NULL;
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	Table table = buildExponentialTable();
	#pragma omp target map(to: table) map(alloc: table_d)
	{
		table_d = (Table*)malloc(sizeof(Table));
		#pragma omp target update to(table_d[0:1])
		memcpy(table_d, &table, sizeof(Table));
	}
	#endif

	// Setup OpenMP threads 
	omp_set_num_threads(I.nthreads);

	// Setup OpenMP RNG on Device.  This section needs significant rewriting for OpenMP target offload.
        //The CUDA curand library is not directly compatible.  A replacement RNG will be needed.
	printf("Setting up OpenMP RNG...\n");
        //curandState * RNG_states; //Remove CUDA RNG state
        //CUDA_CALL( cudaMalloc((void **)&RNG_states, I.streams * sizeof(curandState)) ); //Remove CUDA memory allocation
        //setup_kernel<<<I.streams/100 + 1, 100>>>(RNG_states, I); //Remove CUDA kernel launch
        //CudaCheckError();
        //cudaDeviceSynchronize(); //Remove CUDA synchronization
	
	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
	#pragma omp target map(alloc: flux_states[0:N_flux_states*I.egroups])
	{
		flux_states = (float*)malloc(N_flux_states * I.egroups * sizeof(float));
		#pragma omp target map(tofrom: flux_states[0:N_flux_states*I.egroups])
		{
			init_flux_states<<< blocks, I.egroups >>> ( flux_states, N_flux_states, I, RNG_states ); //This kernel needs to be replaced with OpenMP implementation
		}
	}


	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();
	//cudaDeviceSynchronize(); //Remove CUDA synchronization
	printf("Attentuating fluxes across segments...\n");


	// OpenMP timer variables
	double start_time, end_time;

	// Setup kernel call parameters. This section needs to be rewritten for OpenMP.  The CUDA grid and block dimensions do not map directly to OpenMP.

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
	#pragma omp target teams distribute parallel for \
		map(to: I, sources_d[0:I.source_3D_regions], SA_d, table_d, flux_states[0:N_flux_states*I.egroups]) \
		map(from: flux_states[0:N_flux_states*I.egroups])
	for(int i=0; i < I.segments / I.seg_per_thread; ++i){
		//Call the run_kernel, which needs to be translated to an OpenMP kernel
		run_kernel(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states); //This kernel needs to be replaced with OpenMP implementation

	}
	end_time = omp_get_wtime();

	float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
	#pragma omp target update from(flux_states[0:N_flux_states*I.egroups])
	memcpy(host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float));

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

	return 0;
}