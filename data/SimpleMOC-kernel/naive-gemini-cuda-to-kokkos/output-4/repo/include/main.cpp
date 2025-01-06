#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>

//Structure to hold Kokkos::View
struct KokkosData {
    Kokkos::View<Source*> sources;
    Kokkos::View<float*> fine_flux_arr;
    Kokkos::View<float*> fine_source_arr;
    Kokkos::View<float*> sigT_arr;
    Kokkos::View<float*> flux_states;
    Kokkos::View<curandState*> RNG_states;
    Kokkos::View<Table> table;

};


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
    Source_Arrays SA_h;
    Source* sources_h = initialize_sources(I, &SA_h);


    //Kokkos Space
    Kokkos::InitArguments args;
    args.device_id = 0; //Set default device, can be modified from CLI
    Kokkos::initialize(args);


    KokkosData kokkos_data;
    kokkos_data.sources = Kokkos::View<Source*>("sources", I.source_3D_regions);
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    kokkos_data.fine_source_arr = Kokkos::View<float*>("fine_source_arr", N_fine);
    kokkos_data.fine_flux_arr = Kokkos::View<float*>("fine_flux_arr", N_fine);
    long N_sigT = I.source_3D_regions * I.egroups;
    kokkos_data.sigT_arr = Kokkos::View<float*>("sigT_arr", N_sigT);
    int N_flux_states = 10000;
    kokkos_data.flux_states = Kokkos::View<float*>("flux_states", N_flux_states * I.egroups);
    kokkos_data.RNG_states = Kokkos::View<curandState*>("RNG_states", I.streams);

    //Copy data from host to Kokkos Views
    Kokkos::deep_copy(kokkos_data.sources, sources_h);
    Kokkos::deep_copy(kokkos_data.fine_source_arr, SA_h.fine_source_arr);
    Kokkos::deep_copy(kokkos_data.fine_flux_arr, SA_h.fine_flux_arr);
    Kokkos::deep_copy(kokkos_data.sigT_arr, SA_h.sigT_arr);
    


    // Build Exponential Table
    Kokkos::View<Table> table_h("table_h", 1);
#ifdef TABLE
    printf("Building Exponential Table...\n");
    table_h() = buildExponentialTable();
    kokkos_data.table = Kokkos::create_mirror_view(Kokkos::DefaultExecutionSpace{},table_h);
    Kokkos::deep_copy(kokkos_data.table, table_h());
#endif

    // Setup Kokkos execution space and policy
    Kokkos::DefaultExecutionSpace execution_space;
    Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace> policy(0, I.segments/I.seg_per_thread);

    // Setup Kokkos RNG on Device (replace with Kokkos RNG implementation)
    printf("Setting up Kokkos RNG...\n");
    Kokkos::parallel_for(policy, KOKKOS_LAMBDA(int i){
        curand_init(1234, i, 0, &kokkos_data.RNG_states(i));
    });
    //Kokkos::fence();


    // Allocate Some Flux State vectors to randomly pick from
    printf("Setting up Flux State Vectors...\n");
    Kokkos::parallel_for(policy, KOKKOS_LAMBDA(int i){
        for(int j = 0; j < I.egroups; j++){
            kokkos_data.flux_states(i * I.egroups + j) = curand_uniform(&kokkos_data.RNG_states(i));
        }
    });
    //Kokkos::fence();

    printf("Initialization Complete.\n");
    border_print();
    center_print("SIMULATION", 79);
    border_print();
    //Kokkos::fence();
    printf("Attentuating fluxes across segments...\n");

    // Kokkos timer variables
    Kokkos::Timer timer;
    timer.start();

    // Run Simulation Kernel Loop
    Kokkos::parallel_for(policy,KOKKOS_LAMBDA (const int i){
        run_kernel(I, kokkos_data.sources, kokkos_data.fine_source_arr, kokkos_data.fine_flux_arr, kokkos_data.sigT_arr, kokkos_data.table, kokkos_data.RNG_states,kokkos_data.flux_states, N_flux_states, i);
        });
    Kokkos::fence();
    timer.stop();

    Kokkos::View<float*> host_flux_states("host_flux_states", N_flux_states * I.egroups);
    Kokkos::deep_copy(host_flux_states, kokkos_data.flux_states);

    double time = timer.seconds();

    printf("Simulation Complete.\n");

    border_print();
    center_print("RESULTS SUMMARY", 79);
    border_print();

    double tpi = ((double)(time) / (double)I.segments / (double)I.egroups) * 1.0e9;
    printf("%-25s%.3f seconds\n", "Runtime:", time);
    printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
    border_print();

    Kokkos::finalize();
    return 0;
}