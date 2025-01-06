#include "SimpleMOC-kernel_header.hpp"
#include <Kokkos_Core.hpp>

// Define Kokkos execution space
using Kokkos::DefaultExecutionSpace;

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
    // Allocate device memory for sources using Kokkos
    Kokkos::View<Source*, Kokkos::DefaultExecutionSpace> sources_d("sources_d", I.source_3D_regions);
    initialize_device_sources(I, &SA_h, &SA_d, sources_h, sources_d);


    // Build Exponential Table
    Kokkos::View<Table*> table_d("table_d",1); //Kokkos view for the table
    #ifdef TABLE
    printf("Building Exponential Table...\n");
    Table table = buildExponentialTable();
    table_d() = new Table;
    *table_d() = table;
    #endif

    // Setup Kokkos team policy for parallel execution
    // Mimicking CUDA's grid and block setup as closely as possible
    int n_blocks = std::sqrt(I.segments);
    dim3 blocks(n_blocks, n_blocks);
    if (blocks.x * blocks.y < I.segments) blocks.x++;
    if (blocks.x * blocks.y < I.segments) blocks.y++;
    assert(blocks.x * blocks.y >= I.segments);


    Kokkos::TeamPolicy<Kokkos::DefaultExecutionSpace> team_policy(blocks.x * blocks.y, Kokkos::AUTO);

    // Setup Kokkos RNG on Device.  Curand not directly supported by Kokkos.
    // Replacement needed with Kokkos RNG or another suitable library.
    printf("Setting up Kokkos RNG...\n");
    Kokkos::View<Kokkos::rand::XorShift64Star*,Kokkos::DefaultExecutionSpace> rng_states("rng_states", I.streams);
    Kokkos::parallel_for("setup_rng", team_policy, [&](const Kokkos::TeamMember& teamMember)
    {
        int i = teamMember.team_rank();
        if (i < I.streams)
            rng_states(i) = Kokkos::rand::XorShift64Star(1234 + i); // init RNG. Replace 1234 with better seed.
    });

    // Allocate Some Flux State vectors to randomly pick from
    printf("Setting up Flux State Vectors...\n");
    Kokkos::View<float*,Kokkos::DefaultExecutionSpace> flux_states("flux_states", N_flux_states * I.egroups);
    init_flux_states(flux_states, N_flux_states, I, rng_states);


    printf("Initialization Complete.\n");
    border_print();
    center_print("SIMULATION", 79);
    border_print();

    printf("Attentuating fluxes across segments...\n");

    // Kokkos timer
    Kokkos::Timer timer;

    // Setup kernel call team policy parameters
    assert(I.segments % I.seg_per_thread == 0);
    n_blocks = std::sqrt(I.segments / I.seg_per_thread);
    dim3 blocks_k(n_blocks, n_blocks);
    if (blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread) blocks_k.x++;
    if (blocks_k.x * blocks_k.y < I.segments / I.seg_per_thread) blocks_k.y++;
    assert(blocks_k.x * blocks_k.y >= I.segments / I.seg_per_thread);


    Kokkos::TeamPolicy<Kokkos::DefaultExecutionSpace> team_policy_k(blocks_k.x * blocks_k.y, Kokkos::AUTO);


    // Run Simulation Kernel Loop using Kokkos parallel_for
    Kokkos::parallel_for("run_kernel", team_policy_k, [&](const Kokkos::TeamMember& teamMember)
    {
        run_kernel(I, sources_d, SA_d, table_d, rng_states, flux_states);
    });

    timer.stop();

    printf("Simulation Complete.\n");

    border_print();
    center_print("RESULTS SUMMARY", 79);
    border_print();

    double time = timer.seconds();
    double tpi = ((double)(time) / (double)I.segments / (double)I.egroups) * 1.0e9;
    printf("%-25s%.3f seconds\n", "Runtime:", time);
    printf("%-25s%.8lf ns\n", "Time per Intersection:", tpi);
    border_print();

    //Deallocation of Kokkos views and dynamically allocated memory
    delete table_d();

    return 0;
}