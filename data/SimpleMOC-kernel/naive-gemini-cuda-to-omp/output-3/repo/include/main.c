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
	#pragma omp target enter data map(alloc: table[0:1])
	#pragma omp target enter data map(to: table[0:1])
	table_d = &table;
	#endif

	// Setup OpenMP offload blocks / threads.  Note that we're not directly 
        // mapping CUDA threads/blocks to OpenMP threads.  OpenMP will handle
        // the work distribution.  The `seg_per_thread` parameter will be 
        // used to control workload distribution. We'll adjust the number of
        // OpenMP threads to match the number of segments/seg_per_thread.

        int num_teams = (I.segments + I.seg_per_thread -1)/ I.seg_per_thread;


	// Setup OpenMP RNG.  Replace CUDA RNG with OpenMP RNG.

	printf("Setting up OpenMP RNG...\n");
        //Allocate space for RNG states on host and device
        unsigned int *rng_states_h = (unsigned int*) malloc(num_teams * sizeof(unsigned int));
        unsigned int *rng_states_d;
        #pragma omp target enter data map(alloc: rng_states_d[0:num_teams])

        #pragma omp parallel for num_teams(num_teams)
        for (int i = 0; i < num_teams; i++) {
            rng_states_h[i] = i+1234; //Seed RNGs differently
        }
        #pragma omp target enter data map(to: rng_states_h[0:num_teams]) map(from:rng_states_d[0:num_teams])
        #pragma omp target update from(rng_states_d[0:num_teams])

	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
        #pragma omp target enter data map(alloc: flux_states[0:N_flux_states*I.egroups])
	

	#pragma omp target teams distribute parallel for num_teams(num_teams) thread_limit(I.egroups)
	init_flux_states( flux_states, N_flux_states, I, rng_states_d );


	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();
	printf("Attentuating fluxes across segments...\n");

	// OpenMP timer variables
	double start_time, end_time;
	start_time = omp_get_wtime();

	// Run Simulation Kernel Loop
	#pragma omp target teams distribute parallel for num_teams(num_teams) thread_limit(I.egroups)
	run_kernel(I, sources_d, SA_d, table_d, rng_states_d, flux_states, N_flux_states);

	end_time = omp_get_wtime();
	double time = end_time - start_time;

	float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target update from(flux_states[0:N_flux_states*I.egroups])
	memcpy(host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float));
        #pragma omp target exit data map(delete:flux_states[0:N_flux_states*I.egroups])
        #pragma omp target exit data map(delete:rng_states_d[0:num_teams])
        #pragma omp target exit data map(delete: table_d[0:1])



	printf("Simulation Complete.\n");

	border_print();
	center_print("RESULTS SUMMARY", 79);
	border_print();

	double tpi = ((double) (time) /
			(double)I.segments / (double) I.egroups) * 1.0e9;
	printf("%-25s%.3f seconds\n", "Runtime:", time);
	printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
	border_print();
        free(rng_states_h);
        free(host_flux_states);
	free(sources_h);
	free(SA_h.fine_flux_arr);
	free(SA_h.fine_source_arr);
	free(SA_h.sigT_arr);
	return 0;
}