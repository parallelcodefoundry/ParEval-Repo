#include "SimpleMOC-kernel_header.h"

#include <omp.h>

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

#pragma omp target enter data map(alloc: SA_h.fine_flux_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_h.fine_source_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_h.sigT_arr[0:I.source_3D_regions * I.egroups]) map(to: sources_h[0:I.source_3D_regions])
        {
	sources_d = initialize_device_sources( I, &SA_h, &SA_d, sources_h); 
        }
#pragma omp target exit data map(from: SA_d.fine_flux_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_d.fine_source_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_d.sigT_arr[0:I.source_3D_regions * I.egroups], sources_d[0:I.source_3D_regions]) map(release: SA_h.fine_flux_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_h.fine_source_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_h.sigT_arr[0:I.source_3D_regions * I.egroups], sources_h[0:I.source_3D_regions])


	// Build Exponential Table
	Table * table_d = NULL;
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	Table table = buildExponentialTable();
        
        #pragma omp target enter data map(alloc: table_d[0:1])
        {
	  table_d = (Table *) malloc(sizeof(Table));
	  memcpy(table_d, &table, sizeof(Table));
        }
        #pragma omp target exit data map(release: table_d[0:1])
	#endif

	// Setup CUDA blocks / threads (This part will be handled differently in OpenMP offloading)
	int n_blocks = sqrt(I.segments);
	dim3 blocks(n_blocks, n_blocks);
	if( blocks.x * blocks.y < I.segments )
		blocks.x++;
	if( blocks.x * blocks.y < I.segments )
		blocks.y++;
	assert( blocks.x * blocks.y >= I.segments );

	// Setup CUDA RNG on Device (This will be adapted to OpenMP)
	printf("Setting up RNG...\n");
	curandState * RNG_states;
        #pragma omp target enter data map(alloc: RNG_states[0:I.streams])
        {
          RNG_states = (curandState *) malloc(I.streams * sizeof(curandState));
          setup_kernel(RNG_states, I);
        }
        #pragma omp target exit data map(from: RNG_states[0:I.streams])


	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
        #pragma omp target enter data map(alloc: flux_states[0:N_flux_states * I.egroups])
        {
          flux_states = (float *) malloc(N_flux_states * I.egroups * sizeof(float));
          init_flux_states( flux_states, N_flux_states, I, RNG_states );
        }
        #pragma omp target exit data map(from: flux_states[0:N_flux_states * I.egroups])


	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();

	printf("Attentuating fluxes across segments...\n");

	// Timer variables (Adapt to OpenMP)
	double start_time, end_time;
	start_time = omp_get_wtime();

	// Setup kernel call parameters (Adapt to OpenMP)
	assert( I.segments % I.seg_per_thread == 0 );
	n_blocks = sqrt(I.segments / I.seg_per_thread);
	dim3 blocks_k(n_blocks, n_blocks);
	if( blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread )
		blocks_k.x++;
	if( blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread )
		blocks_k.y++;
	assert( blocks_k.x * blocks_k.y >= I.segments / I.seg_per_thread );

	// Run Simulation Kernel (OpenMP offloading)
        #pragma omp target teams distribute parallel for map(to:I, sources_d[0:I.source_3D_regions], SA_d, table_d, RNG_states[0:I.streams], flux_states[0:N_flux_states*I.egroups])
	for(int i = 0; i < I.segments/I.seg_per_thread; ++i)
        {
            run_kernel(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states);
        }


	end_time = omp_get_wtime();
	double time = end_time - start_time;

	float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target update from(flux_states[0:N_flux_states * I.egroups])
        {
          memcpy(host_flux_states, flux_states, N_flux_states * I.egroups * sizeof(float));
        }

	printf("Simulation Complete.\n");

	border_print();
	center_print("RESULTS SUMMARY", 79);
	border_print();

	double tpi = ((double) (time) /
			(double)I.segments / (double) I.egroups) * 1.0e9;
	printf("%-25s%.3f seconds\n", "Runtime:", time);
	printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
	border_print();

        #pragma omp target exit data map(release: flux_states[0:N_flux_states * I.egroups], RNG_states[0:I.streams], SA_d.fine_flux_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_d.fine_source_arr[0:I.source_3D_regions * I.fine_axial_intervals * I.egroups], SA_d.sigT_arr[0:I.source_3D_regions * I.egroups], sources_d[0:I.source_3D_regions])
	free(host_flux_states);

	return 0;
}