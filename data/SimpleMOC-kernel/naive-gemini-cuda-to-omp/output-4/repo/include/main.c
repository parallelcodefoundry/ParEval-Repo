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

        #pragma omp target enter data map(alloc: SA_h.fine_flux_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_h.fine_source_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_h.sigT_arr[0:I.source_3D_regions * I.egroups])
        #pragma omp target enter data map(to: sources_h[0:I.source_3D_regions])

	sources_d = initialize_device_sources( I, &SA_h, &SA_d, sources_h); 
	//cudaDeviceSynchronize(); //No need for this in OpenMP offloading

	// Build Exponential Table
	Table * table_d = NULL;
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	Table table = buildExponentialTable();
        #pragma omp target enter data map(alloc: table_d[0:1])
	#pragma omp target update to(table_d[0:1])
	#pragma omp target update from(table)
	#endif

	// Setup OpenMP offloading  blocks / threads -  simplified for demonstration
	int n_blocks = sqrt(I.segments);


	// Setup OpenMP RNG on Device - simplified for demonstration.  Proper RNG needs more work.
	printf("Setting up OpenMP RNG...\n");


	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
        #pragma omp target enter data map(alloc: flux_states[0:N_flux_states * I.egroups])

	init_flux_states<<< n_blocks, I.egroups >>> ( flux_states, N_flux_states, I, NULL ); //NULL for RNG state as simplified here


	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();
	//cudaDeviceSynchronize(); //No need for this in OpenMP offloading
	printf("Attentuating fluxes across segments...\n");

	// OpenMP timer variables
	double start, stop;
	start = omp_get_wtime();
	
	// Setup kernel call block parameters
	assert( I.segments % I.seg_per_thread == 0 );
	n_blocks = sqrt(I.segments / I.seg_per_thread);


	// Run Simulation Kernel Loop
	#pragma omp target teams distribute parallel num_teams(n_blocks) thread_limit(I.egroups)
	{
		run_kernel(I, sources_d, SA_d, table_d, NULL, flux_states, N_flux_states);
	}

	stop = omp_get_wtime();
	double time = stop - start;

	float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target update from(flux_states[0:N_flux_states * I.egroups])

	printf("Simulation Complete.\n");

	border_print();
	center_print("RESULTS SUMMARY", 79);
	border_print();

	double tpi = ((double) (time) /
			(double)I.segments / (double) I.egroups) * 1.0e9;
	printf("%-25s%.3f seconds\n", "Runtime:", time);
	printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
	border_print();

        #pragma omp target exit data map(release: SA_h.fine_flux_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_h.fine_source_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_h.sigT_arr[0:I.source_3D_regions * I.egroups], sources_h[0:I.source_3D_regions],flux_states[0:N_flux_states * I.egroups])
        #ifdef TABLE
        #pragma omp target exit data map(release: table_d[0:1])
        #endif


	return 0;
}