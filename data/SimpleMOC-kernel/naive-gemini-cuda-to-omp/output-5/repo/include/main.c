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
	#pragma omp target map(to:table[0:1])
	{
		table_d = &table;
	}
	#pragma omp target exit data map(from:table[0:1])
	#endif

	// Setup OpenMP threads
	omp_set_num_threads(I.nthreads);

	// Setup OpenMP RNG on Device -  This section needs significant changes to work with OpenMP
	printf("Setting up OpenMP RNG...\n");
	curandState * RNG_states;
	#pragma omp target enter data map(alloc: RNG_states[0:I.streams])
	//  The following line is problematic.  curand_init requires a single random number generator seed per thread, but this code is seeding many with a single seed.
    // setup_kernel<<<I.streams/100 + 1, 100>>>(RNG_states, I);
	#pragma omp parallel for
	for (int i = 0; i < I.streams; i++) {
		curand_init(1234 + i, i, 0, &RNG_states[i]);
	}
	#pragma omp target update from(RNG_states[0:I.streams])

	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
	#pragma omp target enter data map(alloc: flux_states[0:N_flux_states*I.egroups])

	#pragma omp target map(tofrom: flux_states[0:N_flux_states*I.egroups])
	init_flux_states<<<1, N_flux_states>>>(flux_states, N_flux_states, I, RNG_states);

	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();

	printf("Attentuating fluxes across segments...\n");

	// OpenMP timer variables
	double start_time, end_time;
	start_time = omp_get_wtime();
	
	// Setup kernel call block parameters.  This part needs adjustment for OpenMP
	assert( I.segments % I.seg_per_thread == 0 );
	int n_blocks = I.segments / I.seg_per_thread; // Simplified block calculation for OpenMP

	#pragma omp target teams distribute parallel for map(tofrom: flux_states[0:N_flux_states*I.egroups])
	for(int blockId = 0; blockId < n_blocks; ++blockId) {
		run_kernel(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states, blockId);
	}

	end_time = omp_get_wtime();
	double time = end_time - start_time;

	float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
	#pragma omp target update from(flux_states[0:N_flux_states * I.egroups])
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

	#pragma omp target exit data map(release: flux_states[0:N_flux_states*I.egroups], RNG_states[0:I.streams])

	free(host_flux_states);

	return 0;
}