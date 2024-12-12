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
	if (posix_memalign((void**)&table_d, 64, sizeof(Table)) != 0) {
            perror("posix_memalign failed");
            exit(1);
        }
	#pragma omp target enter data map(to: table[0:1])
        #pragma omp target update to(table_d[0:1])
	#endif

	// Setup OpenMP threads
	omp_set_num_threads(I.nthreads);

	// Setup OpenMP RNG -  Replace CUDA RNG with OpenMP RNG
	printf("Setting up OpenMP RNG...\n");
        unsigned int * RNG_states;
        if (posix_memalign((void**)&RNG_states, 64, I.streams * sizeof(unsigned int)) != 0) {
            perror("posix_memalign failed");
            exit(1);
        }
        #pragma omp target enter data map(to: RNG_states[0:I.streams])
        
        #pragma omp parallel for
        for (int i = 0; i < I.streams; i++) {
            RNG_states[i] = i; //Simple initialization, replace with proper OpenMP RNG
        }
        #pragma omp target update to(RNG_states[0:I.streams])


	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	float * flux_states;
	int N_flux_states = 10000;
	assert( I.segments >= N_flux_states );
        if (posix_memalign((void**)&flux_states, 64, N_flux_states * I.egroups * sizeof(float)) != 0) {
            perror("posix_memalign failed");
            exit(1);
        }
	#pragma omp target enter data map(to: flux_states[0:N_flux_states*I.egroups])
	init_flux_states<<< blocks, I.egroups >>> ( flux_states, N_flux_states, I, RNG_states );


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
	dim3 blocks_k(n_blocks, n_blocks);
	if( blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread )
		blocks_k.x++;
	if( blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread )
		blocks_k.y++;
	assert( blocks_k.x * blocks_k.y >= I.segments / I.seg_per_thread );

	// Run Simulation Kernel Loop
	#pragma omp target teams distribute parallel for
	run_kernel(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states);

	end_time = omp_get_wtime();
	double time = end_time - start_time;

	float * host_flux_states = (float*) malloc(N_flux_states * I.egroups * sizeof(float));
        #pragma omp target update from(flux_states[0:N_flux_states*I.egroups])
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

        #pragma omp target exit data map(delete: flux_states, RNG_states, table_d, sources_d, SA_d.fine_flux_arr, SA_d.fine_source_arr, SA_d.sigT_arr)
        free(sources_h);
        free(SA_h.fine_flux_arr);
        free(SA_h.fine_source_arr);
        free(SA_h.sigT_arr);
        free(host_flux_states);
        free(table_d);
        free(RNG_states);
        free(sources_d);

	return 0;
}