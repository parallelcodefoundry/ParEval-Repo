#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>

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

  //Kokkos::Space::DeviceSpace deviceSpace;
	// Allocate Kokkos device memory
  Kokkos::View<Source*, Kokkos::DefaultExecutionSpace::device_type> sources_d("sources_d", I.source_3D_regions);
  Kokkos::View<float*, Kokkos::DefaultExecutionSpace::device_type> fine_source_d("fine_source_d", N_fine);
  Kokkos::View<float*, Kokkos::DefaultExecutionSpace::device_type> fine_flux_d("fine_flux_d", N_fine);
  Kokkos::View<float*, Kokkos::DefaultExecutionSpace::device_type> sigT_d("sigT_d", N_sigT);

	//Copy data to kokkos device
  Kokkos::deep_copy(sources_d, sources_h);
  Kokkos::deep_copy(fine_source_d, SA_h.fine_source_arr);
  Kokkos::deep_copy(fine_flux_d, SA_h.fine_flux_arr);
  Kokkos::deep_copy(sigT_d, SA_h.sigT_arr);

	// Build Exponential Table
	Kokkos::View<Table*, Kokkos::DefaultExecutionSpace::device_type> table_d("table_d", 1);
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	Table table = buildExponentialTable();
	Kokkos::deep_copy(table_d, &table);
	#endif

	// Setup Kokkos execution space for the GPU
  Kokkos::DefaultExecutionSpace execution_space;

	// Setup Kokkos RNG on Device (replace with Kokkos RNG implementation)
	printf("Setting up Kokkos RNG...\n");
	Kokkos::View<curandState*, Kokkos::DefaultExecutionSpace::device_type> RNG_states("RNG_states", I.streams);
	Kokkos::parallel_for("setup_kernel", Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(execution_space, 0, I.streams), [&](int i){
		curand_init(1234, i, 0, &RNG_states[i]);
	});


	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	Kokkos::View<float*, Kokkos::DefaultExecutionSpace::device_type> flux_states("flux_states", N_flux_states * I.egroups);
  Kokkos::parallel_for("init_flux_states", Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(execution_space, 0, N_flux_states), [&](int i){
    for(int j=0; j< I.egroups; ++j){
      flux_states[i*I.egroups + j] = curand_uniform(&RNG_states[i % I.streams]);
    }
  });

	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();

	printf("Attentuating fluxes across segments...\n");

	// Kokkos timer variables
	Kokkos::Timer timer;

	// Setup kernel call parameters.  Adapt to Kokkos's parallel_for
  int n_blocks = I.segments / I.seg_per_thread; //Adjust as needed for Kokkos policy

	// Run Simulation Kernel Loop
	timer.start();
	Kokkos::parallel_for("run_kernel", Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(execution_space, 0, n_blocks), [&](int i){
    // Adapt run_kernel to Kokkos, likely requiring restructuring
    // Implement shared memory similarly within Kokkos parallel_for
    run_kernel_kokkos(i, I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states);
  });
	timer.stop();

	//Copy results back from device
  Kokkos::View<float*, Kokkos::HostSpace> host_flux_states("host_flux_states", N_flux_states * I.egroups);
  Kokkos::deep_copy(host_flux_states, flux_states);


	printf("Simulation Complete.\n");

	border_print();
	center_print("RESULTS SUMMARY", 79);
	border_print();

	double time = timer.seconds();
	double tpi = ((double) (time) /
			(double)I.segments / (double) I.egroups) * 1.0e9;
	printf("%-25s%.3f seconds\n", "Runtime:", time);
	printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
	border_print();

	return 0;
}