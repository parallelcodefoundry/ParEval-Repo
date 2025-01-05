#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>

int main(int argc, char* argv[]) {
    int version = 4;

    srand(time(NULL));

    Input I = set_default_input();
    read_CLI(argc, argv, &I);

    // Calculate Number of 3D Source Regions
    I.source_3D_regions = (int)ceil((double)I.source_2D_regions *
                                     I.coarse_axial_intervals / I.decomp_assemblies_ax);

    logo(version);

    print_input_summary(I);

    center_print("INITIALIZATION", 79);
    border_print();

    // Build Source Data
    printf("Building Source Data Arrays...\n");
    Source_Arrays SA_h, SA_d;
    Source* sources_h = initialize_sources(I, &SA_h);
    //Using Kokkos::View for device allocation and copy
    Kokkos::View<Source*, Kokkos::DefaultExecutionSpace> sources_d("sources_d", I.source_3D_regions);
    Kokkos::View<float*, Kokkos::DefaultExecutionSpace> fine_source_arr_d("fine_source_arr_d", N_fine);
    Kokkos::View<float*, Kokkos::DefaultExecutionSpace> fine_flux_arr_d("fine_flux_arr_d", N_fine);
    Kokkos::View<float*, Kokkos::DefaultExecutionSpace> sigT_arr_d("sigT_arr_d", N_sigT);


    Kokkos::DeepCopy(sources_d, sources_h);
    Kokkos::DeepCopy(fine_source_arr_d, SA_h.fine_source_arr);
    Kokkos::DeepCopy(fine_flux_arr_d, SA_h.fine_flux_arr);
    Kokkos::DeepCopy(sigT_arr_d, SA_h.sigT_arr);


    // Build Exponential Table
    Kokkos::View<Table*, Kokkos::DefaultExecutionSpace> table_d("table_d", 1);
    #ifdef TABLE
    printf("Building Exponential Table...\n");
    Table table = buildExponentialTable();
    Kokkos::DeepCopy(table_d, &table);
    #endif

    // Setup Kokkos parallel execution policies
    int n_blocks = sqrt(I.segments);
    int team_size = I.egroups;
    int num_teams = (I.segments + team_size -1)/ team_size;


    // Setup Kokkos RNG on Device - replace with Kokkos RNG
    printf("Setting up Kokkos RNG...\n");
    Kokkos::View<curandState*, Kokkos::DefaultExecutionSpace> RNG_states("RNG_states", I.streams);
    Kokkos::parallel_for(Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(0,I.streams), [&](int i){
        curand_init(1234, i, 0, &RNG_states[i]);
    });

    // Allocate Some Flux State vectors to randomly pick from
    printf("Setting up Flux State Vectors...\n");
    Kokkos::View<float*, Kokkos::DefaultExecutionSpace> flux_states("flux_states", N_flux_states * I.egroups);
    Kokkos::parallel_for(Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(0, N_flux_states * I.egroups),[&](int i){
        flux_states[i] = (float)rand()/RAND_MAX;
    });


    printf("Initialization Complete.\n");
    border_print();
    center_print("SIMULATION", 79);
    border_print();

    printf("Attentuating fluxes across segments...\n");

    // Kokkos timer variables
    Kokkos::Timer timer;

    // Run Simulation Kernel Loop
    timer.start();
    Kokkos::parallel_for(Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(0, num_teams), [&](int team_id){
        run_kernel(I, sources_d, SA_d, table_d, RNG_states, flux_states, N_flux_states, team_id, team_size);
    });
    timer.stop();

    Kokkos::View<float*, Kokkos::HostSpace> host_flux_states("host_flux_states", N_flux_states * I.egroups);
    Kokkos::DeepCopy(host_flux_states, flux_states);

    printf("Simulation Complete.\n");

    border_print();
    center_print("RESULTS SUMMARY", 79);
    border_print();

    double time = timer.seconds();
    double tpi = ((double)(time) / (double)I.segments / (double)I.egroups) * 1.0e9;
    printf("%-25s%.3f seconds\n", "Runtime:", time);
    printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
    border_print();

    return 0;
}