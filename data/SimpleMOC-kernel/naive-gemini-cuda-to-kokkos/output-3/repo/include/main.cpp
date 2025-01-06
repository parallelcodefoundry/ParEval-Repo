#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>

//Kokkos::CudaSpace for using CUDA
using KokkosSpace = Kokkos::CudaSpace;


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
    //Allocate Kokkos::View for SA_d
    Kokkos::View<Source*, KokkosSpace> sources_d("sources_d", I.source_3D_regions);
    Kokkos::View<float*, KokkosSpace> fine_source_arr_d("fine_source_arr_d", N_fine);
    Kokkos::View<float*, KokkosSpace> fine_flux_arr_d("fine_flux_arr_d", N_fine);
    Kokkos::View<float*, KokkosSpace> sigT_arr_d("sigT_arr_d", N_sigT);
    SA_d.fine_source_arr = fine_source_arr_d.data();
    SA_d.fine_flux_arr = fine_flux_arr_d.data();
    SA_d.sigT_arr = sigT_arr_d.data();
    //Copy data to device with Kokkos::deep_copy
    Kokkos::deep_copy(sources_d, sources_h);
    Kokkos::deep_copy(fine_source_arr_d, SA_h.fine_source_arr);
    Kokkos::deep_copy(fine_flux_arr_d, SA_h.fine_flux_arr);
    Kokkos::deep_copy(sigT_arr_d, SA_h.sigT_arr);

    // Build Exponential Table
    Kokkos::View<Table, KokkosSpace> table_d("table_d",1);
    #ifdef TABLE
    printf("Building Exponential Table...\n");
    Table table = buildExponentialTable();
    Kokkos::deep_copy(table_d, table);
    #endif

    // Setup Kokkos::parallel_for
    int n_blocks = sqrt(I.segments);
    dim3 blocks(n_blocks, n_blocks);
    if (blocks.x * blocks.y < I.segments)
        blocks.x++;
    if (blocks.x * blocks.y < I.segments)
        blocks.y++;
    assert(blocks.x * blocks.y >= I.segments);

    //Kokkos RNG is different, requires separate implementation
    printf("Setting up Kokkos RNG...\n");
    Kokkos::View<curandState*, KokkosSpace> RNG_states("RNG_states", I.streams);
    Kokkos::parallel_for("setup_kernel", I.streams, [&](int i) {
        curand_init(1234, i, 0, &RNG_states(i));
    });
    Kokkos::fence();


    // Allocate Some Flux State vectors to randomly pick from
    printf("Setting up Flux State Vectors...\n");
    Kokkos::View<float*, KokkosSpace> flux_states("flux_states", N_flux_states * I.egroups);
    Kokkos::parallel_for("init_flux_states", Kokkos::MDRangePolicy<KokkosSpace, Kokkos::Rank<2>>(blocks, I.egroups),[&](int i, int j){
        int blockId = i;
        curandState* localState = &RNG_states[blockId % I.streams];
        flux_states(blockId, j) = curand_uniform(localState);
    });
    Kokkos::fence();


    printf("Initialization Complete.\n");
    border_print();
    center_print("SIMULATION", 79);
    border_print();

    printf("Attentuating fluxes across segments...\n");

    // Kokkos timer
    Kokkos::Timer timer;

    // Setup kernel call parameters.  Kokkos handles the block/thread management differently.
    assert(I.segments % I.seg_per_thread == 0);
    n_blocks = sqrt(I.segments / I.seg_per_thread);
    dim3 blocks_k(n_blocks, n_blocks);
    if (blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread)
        blocks_k.x++;
    if (blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread)
        blocks_k.y++;
    assert(blocks_k.x * blocks_k.y >= I.segments / I.seg_per_thread);

    //Run Kokkos kernel
    Kokkos::parallel_for("run_kernel", Kokkos::MDRangePolicy<KokkosSpace, Kokkos::Rank<2>>(blocks_k, I.egroups),[&](int i, int j){
        //Implement Kokkos version of run_kernel here.  Will require significant restructuring.
        //This is a placeholder and will not compile.  See comments below for details.
    });
    Kokkos::fence();
    double time = timer.seconds();


    //Copy data back to host (if needed)
    Kokkos::View<float*, Kokkos::HostSpace> host_flux_states("host_flux_states", N_flux_states * I.egroups);
    Kokkos::deep_copy(host_flux_states, flux_states);


    printf("Simulation Complete.\n");

    border_print();
    center_print("RESULTS SUMMARY", 79);
    border_print();

    double tpi = ((double)(time) / (double)I.segments / (double)I.egroups) * 1.0e9;
    printf("%-25s%.3f seconds\n", "Runtime:", time);
    printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
    border_print();

    return 0;
}