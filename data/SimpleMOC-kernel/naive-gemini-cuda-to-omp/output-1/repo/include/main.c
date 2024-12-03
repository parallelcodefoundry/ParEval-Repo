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
	//cudaDeviceSynchronize(); //removed - no CUDA here

	// Build Exponential Table
	Table * table_d = NULL;
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	Table table = buildExponentialTable();
        // The following lines are replaced with omp target data
	#pragma omp target enter data map(to:table[0:1])
	#pragma omp target data map(alloc:table_d[0:1])
	{
	#pragma omp target update to(table_d[0:1])
	}

	#endif

	// Setup OpenMP offload
        int n_blocks = sqrt(I.segments);
        dim3 blocks(n_blocks, n_blocks);
        if( blocks.x * blocks.y < I.segments )
                blocks.x++;
        if( blocks.x * blocks.y < I.segments )
                blocks.y++;
        assert( blocks.x * blocks.y >= I.segments );


	// Setup OpenMP RNG on Device
	printf("Setting up OpenMP RNG...\n");
	curandState * RNG_states;
        // Allocate RNG states on the host
	RNG_states = (curandState*)malloc(I.streams * sizeof(curandState));
        // Initialize RNG states on the host, this can be parallelized with OpenMP
        #pragma omp parallel for
        for (int i = 0; i < I.streams; i++) {
            curand_init(1234, i, 0, &RNG_states[i]);
        }
	//cudaDeviceSynchronize(); //removed - no CUDA here


	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
	flux_states = (float*)malloc(N_flux_states * I.egroups * sizeof(float));
        // Initialize flux states on the host, this can be parallelized with OpenMP
        #pragma omp parallel for
        for (int i = 0; i < N_flux_states * I.egroups; i++) {
            flux_states[i] = (float)rand() / RAND_MAX;
        }

	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();
	//cudaDeviceSynchronize(); //removed - no CUDA here
	printf("Attentuating fluxes across segments...\n");

	// Timer variables
	double start_time, end_time;
	
	// Setup kernel call parameters
	assert( I.segments % I.seg_per_thread == 0 );
	n_blocks = sqrt(I.segments / I.seg_per_thread);
	dim3 blocks_k(n_blocks, n_blocks);
	if( blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread )
		blocks_k.x++;
	if( blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread )
		blocks_k.y++;
	assert( blocks_k.x * blocks_k.y >= I.segments / I.seg_per_thread );

	// Run Simulation Kernel Loop
        start_time = omp_get_wtime();
	#pragma omp target teams distribute parallel for \
	map(to: I, sources_d[0:I.source_3D_regions], SA_d, table_d, RNG_states[0:I.streams], flux_states[0:N_flux_states*I.egroups]) \
        map(from:flux_states[0:N_flux_states*I.egroups])
	for(int i = 0; i < I.segments/I.seg_per_thread; i++) {
                run_kernel<<<1, I.egroups>>>(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states);
        }

	end_time = omp_get_wtime();
	double time = end_time - start_time;

	printf("Simulation Complete.\n");

	border_print();
	center_print("RESULTS SUMMARY", 79);
	border_print();

	double tpi = ((double) (time) /
			(double)I.segments / (double) I.egroups) * 1.0e9;
	printf("%-25s%.3f seconds\n", "Runtime:", time);
	printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
	border_print();

	free(flux_states);
        free(RNG_states);
        #pragma omp target exit data map(from:table_d[0:1])
	#pragma omp target exit data map(release:table_d[0:1])
	free(sources_h);
	free(SA_h.fine_flux_arr);
	free(SA_h.fine_source_arr);
	free(SA_h.sigT_arr);

	return 0;
}