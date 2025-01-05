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

  // Kokkos::View allocation and copy
  Kokkos::View<Source*, Kokkos::HostSpace> sources_h_view(sources_h, I.source_3D_regions);
  Kokkos::View<Source*, Kokkos::CudaSpace> sources_d_view("sources_d", I.source_3D_regions);
  Kokkos::deep_copy(sources_d_view, sources_h_view);


	Source * sources_d = sources_d_view.data(); // Get the device pointer

  // Kokkos::View allocation and copy for other arrays
  Kokkos::View<float*, Kokkos::HostSpace> fine_flux_arr_h_view(SA_h.fine_flux_arr, I.source_3D_regions * I.fine_axial_intervals * I.egroups);
  Kokkos::View<float*, Kokkos::CudaSpace> fine_flux_arr_d_view("fine_flux_arr_d", I.source_3D_regions * I.fine_axial_intervals * I.egroups);
  Kokkos::deep_copy(fine_flux_arr_d_view, fine_flux_arr_h_view);
  SA_d.fine_flux_arr = fine_flux_arr_d_view.data();

  Kokkos::View<float*, Kokkos::HostSpace> fine_source_arr_h_view(SA_h.fine_source_arr, I.source_3D_regions * I.fine_axial_intervals * I.egroups);
  Kokkos::View<float*, Kokkos::CudaSpace> fine_source_arr_d_view("fine_source_arr_d", I.source_3D_regions * I.fine_axial_intervals * I.egroups);
  Kokkos::deep_copy(fine_source_arr_d_view, fine_source_arr_h_view);
  SA_d.fine_source_arr = fine_source_arr_d_view.data();


  Kokkos::View<float*, Kokkos::HostSpace> sigT_arr_h_view(SA_h.sigT_arr, I.source_3D_regions * I.egroups);
  Kokkos::View<float*, Kokkos::CudaSpace> sigT_arr_d_view("sigT_arr_d", I.source_3D_regions * I.egroups);
  Kokkos::deep_copy(sigT_arr_d_view, sigT_arr_h_view);
  SA_d.sigT_arr = sigT_arr_d_view.data();

	// Build Exponential Table
	Kokkos::View<Table, Kokkos::HostSpace> table_h_view;
	Kokkos::View<Table, Kokkos::CudaSpace> table_d_view;
	#ifdef TABLE
	printf("Building Exponential Table...\n");
	table_h_view = Kokkos::View<Table, Kokkos::HostSpace>(buildExponentialTable());
	table_d_view = Kokkos::View<Table, Kokkos::CudaSpace>("table_d", 1); //Size 1 since only one Table
	Kokkos::deep_copy(table_d_view, table_h_view);
	#endif

  // Kokkos::View for RNG states
  Kokkos::View<curandState*, Kokkos::CudaSpace> RNG_states_view("RNG_states", I.streams);
  curandState* RNG_states = RNG_states_view.data();

	// Setup Kokkos parallel RNG on Device using a Kokkos kernel
	printf("Setting up Kokkos RNG...\n");
  Kokkos::parallel_for("setup_kernel", Kokkos::RangePolicy<Kokkos::Cuda>(0, I.streams), 
                       [&](int i){ curand_init(1234, i, 0, &RNG_states[i]);});
  //Kokkos::fence();


	// Allocate Some Flux State vectors to randomly pick from
	printf("Setting up Flux State Vectors...\n");
	Kokkos::View<float*, Kokkos::CudaSpace> flux_states_view("flux_states", I.egroups * 10000); // Adjust size as needed
	float * flux_states = flux_states_view.data();
  Kokkos::parallel_for("init_flux_states", Kokkos::RangePolicy<Kokkos::Cuda>(0, 10000), 
                       [&](int i)
  {
     for(int j = 0; j < I.egroups; ++j){
        flux_states[i * I.egroups + j] = curand_uniform(&RNG_states[i % I.streams]); // Distribute among streams
     }
  });


	printf("Initialization Complete.\n");
	border_print();
	center_print("SIMULATION", 79);
	border_print();

	printf("Attentuating fluxes across segments...\n");

	// Kokkos timer variables
	Kokkos::Timer timer;

	// Setup kernel call parameters.  Adjust as needed for Kokkos::parallel_for
  int n_blocks = (int)ceil( (double)I.segments / I.seg_per_thread);
  int team_size = I.egroups;


	// Run Simulation Kernel Loop
	timer.start();
  Kokkos::parallel_for("run_kernel", Kokkos::RangePolicy<Kokkos::Cuda>(0, n_blocks),
                       [&](int i){
                          run_kernel(I, sources_d, SA_d, table_d_view.data(), RNG_states, flux_states, 10000, i * I.seg_per_thread);
                       });
  Kokkos::fence();
	timer.stop();

	float time = timer.seconds();

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