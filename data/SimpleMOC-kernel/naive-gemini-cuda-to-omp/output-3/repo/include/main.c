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
	#pragma omp target enter data map(to:SA_d,sources_d[0:I.source_3D_regions])
	
	// Build Exponential Table
	Table * table_d = NULL;
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	Table table = buildExponentialTable();
	#pragma omp target enter data map(to:table)
	#pragma omp target allocate(table_d)
	#pragma omp target update to(table_d[0:1])
	#endif

	// Setup OpenMP threads
	omp_set_num_threads(I.nthreads);

	// Setup OpenMP RNG on Device (replace CUDA RNG with OpenMP RNG)
	printf("Setting up OpenMP RNG...\n");
        //OpenMP does not directly have an equivalent to curand.  Need to replace this with an OpenMP compatible RNG.
        //This is a placeholder and may not be suitable for all applications.
	unsigned int * RNG_states;
        RNG_states = (unsigned int *) malloc(I.streams * sizeof(unsigned int));
        for (int i = 0; i < I.streams; i++) {
            RNG_states[i] = i; // Initialize with a simple seed
        }
        #pragma omp target enter data map(to:RNG_states[0:I.streams])



	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
	#pragma omp target allocate(flux_states)
	#pragma omp target update to(flux_states[0:N_flux_states*I.egroups])


	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();

	printf("Attentuating fluxes across segments...\n");

	// OpenMP timer variables
	double start_time, end_time;
	start_time = omp_get_wtime();
	
	// Setup kernel call block parameters
	assert( I.segments % I.seg_per_thread == 0 );
	int n_blocks = sqrt(I.segments / I.seg_per_thread);
	if( n_blocks * n_blocks < I.segments / I.seg_per_thread )
		n_blocks++;
	assert( n_blocks * n_blocks >= I.segments / I.seg_per_thread );


	// Run Simulation Kernel Loop using OpenMP offloading
	#pragma omp target teams distribute parallel for map(tofrom:flux_states[0:N_flux_states*I.egroups])
	for (int blockId = 0; blockId < I.segments / I.seg_per_thread; blockId++) {
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
        #pragma omp target exit data map(release:SA_d,sources_d[0:I.source_3D_regions],flux_states,table_d,RNG_states)
        free(RNG_states);
        free(host_flux_states);


	return 0;
}