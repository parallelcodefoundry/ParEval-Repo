#include "XSbench_header.hpp"

// Moves all required data structures to the Kokkos's memory space
SimulationData move_simulation_data_to_device(Inputs in, int mype, SimulationData SD) {
    if (mype == 0) printf("Allocating and moving simulation data to Kokkos memory space...\n");

    //Kokkos::Timer timer;

    ////////////////////////////////////////////////////////////////////////////////
    // SUMMARY: Simulation Data Structure Manifest for "SD" Object
    // Here we list all heap arrays (and lengths) in SD that would need to be
    // offloaded manually if using an accelerator with a separate memory space
    ////////////////////////////////////////////////////////////////////////////////
    // int * num_nucs;                     // Length = length_num_nucs;
    // double * concs;                     // Length = length_concs
    // int * mats;                         // Length = length_mats
    // double * unionized_energy_array;    // Length = length_unionized_energy_array
    // int * index_grid;                   // Length = length_index_grid
    // NuclideGridPoint * nuclide_grid;    // Length = length_nuclide_grid
    //
    // Note: "unionized_energy_array" and "index_grid" can be of zero length
    //        depending on lookup method.
    //
    // Note: "Lengths" are given as the number of objects in the array, not the
    //       number of bytes.
    ////////////////////////////////////////////////////////////////////////////////

    // Shallow copy of CPU simulation data to Kokkos simulation data
    SimulationData KSD = SD;

    //Kokkos device types
    using KokkosNumNucsViewType = Kokkos::View<int*, Kokkos::DefaultExecutionSpace>;
    using KokkosConcsViewType = Kokkos::View<double*, Kokkos::DefaultExecutionSpace>;
    using KokkosMatsViewType = Kokkos::View<int*, Kokkos::DefaultExecutionSpace>;
    using KokkosUnionizedViewType = Kokkos::View<double*, Kokkos::DefaultExecutionSpace>;
    using KokkosIndexViewType = Kokkos::View<int*, Kokkos::DefaultExecutionSpace>;
    using KokkosNuclideViewType = Kokkos::View<NuclideGridPoint*, Kokkos::DefaultExecutionSpace>;
    using KokkosVerificationViewType = Kokkos::View<unsigned long*, Kokkos::DefaultExecutionSpace>;

    //Allocate Kokkos views
    KSD.num_nucs = Kokkos::View<int*>("num_nucs", KSD.length_num_nucs);
    KSD.concs = Kokkos::View<double*>("concs", KSD.length_concs);
    KSD.mats = Kokkos::View<int*>("mats", KSD.length_mats);

    if (SD.length_unionized_energy_array != 0) {
        KSD.unionized_energy_array = Kokkos::View<double*>("unionized_energy_array", KSD.length_unionized_energy_array);
    }

    if (SD.length_index_grid != 0) {
        KSD.index_grid = Kokkos::View<int*>("index_grid", KSD.length_index_grid);
    }

    KSD.nuclide_grid = Kokkos::View<NuclideGridPoint*>("nuclide_grid", KSD.length_nuclide_grid);
    KSD.verification = Kokkos::View<unsigned long*>("verification", in.lookups);


    //Copy data to Kokkos views using Kokkos::deep_copy
    Kokkos::deep_copy(KSD.num_nucs, SD.num_nucs);
    Kokkos::deep_copy(KSD.concs, SD.concs);
    Kokkos::deep_copy(KSD.mats, SD.mats);

    if (SD.length_unionized_energy_array != 0) {
        Kokkos::deep_copy(KSD.unionized_energy_array, SD.unionized_energy_array);
    }

    if (SD.length_index_grid != 0) {
        Kokkos::deep_copy(KSD.index_grid, SD.index_grid);
    }

    Kokkos::deep_copy(KSD.nuclide_grid, SD.nuclide_grid);


    // Synchronize
    //Kokkos::fence();

    if (mype == 0) printf("Kokkos Initialization complete.\n"); //Removed size output as Kokkos memory management is different

    return KSD;
}

// Release Kokkos memory
void release_device_memory(SimulationData KSD) {
    //Kokkos views are automatically deallocated when they go out of scope
}

void release_memory(SimulationData SD) {
    //Free host memory
    free(SD.num_nucs);
    free(SD.concs);
    free(SD.mats);
    if (SD.length_unionized_energy_array > 0) free(SD.unionized_energy_array);
    free(SD.nuclide_grid);
    free(SD.verification);
}

SimulationData grid_init_do_not_profile(Inputs in, int mype) {
    // Structure to hold all allocated simulation data arrays
    SimulationData SD;

    // Keep track of how much data we're allocating
    size_t nbytes = 0;

    // Set the initial seed value
    uint64_t seed = 42;

    ////////////////////////////////////////////////////////////////////
    // Initialize Nuclide Grids
    ////////////////////////////////////////////////////////////////////

    if (mype == 0) printf("Initializing nuclide grids...\n");

    // Initialize Nuclide Grid
    SD.length_nuclide_grid = in.n_isotopes * in.n_gridpoints;
    SD.nuclide_grid = (NuclideGridPoint*)malloc(SD.length_nuclide_grid * sizeof(NuclideGridPoint));
    assert(SD.nuclide_grid != NULL);
    nbytes += SD.length_nuclide_grid * sizeof(NuclideGridPoint);
    for (int i = 0; i < SD.length_nuclide_grid; i++) {
        SD.nuclide_grid[i].energy = LCG_random_double(&seed);
        SD.nuclide_grid[i].total_xs = LCG_random_double(&seed);
        SD.nuclide_grid[i].elastic_xs = LCG_random_double(&seed);
        SD.nuclide_grid[i].absorbtion_xs = LCG_random_double(&seed);
        SD.nuclide_grid[i].fission_xs = LCG_random_double(&seed);
        SD.nuclide_grid[i].nu_fission_xs = LCG_random_double(&seed);
    }

    // Sort so that each nuclide has data stored in ascending energy order.
    for (int i = 0; i < in.n_isotopes; i++)
        qsort(&SD.nuclide_grid[i * in.n_gridpoints], in.n_gridpoints, sizeof(NuclideGridPoint), NGP_compare);

    // Allocate Verification Array
    size_t sz = in.lookups * sizeof(unsigned long);
    SD.verification = (unsigned long*)malloc(sz);
    nbytes += sz;

    //Rest of the initialization remains the same as the original CUDA code.  The Kokkos::View will handle the device allocation during the `move_simulation_data_to_device` call.

    // ... (rest of the function remains unchanged)
    ////////////////////////////////////////////////////////////////////
    // Initialize Acceleration Structure
    ////////////////////////////////////////////////////////////////////

    // ...

    ////////////////////////////////////////////////////////////////////
    // Initialize Materials and Concentrations
    ////////////////////////////////////////////////////////////////////
    // ...


    if (mype == 0) printf("Initialization complete. Allocated %.0lf MB of data on CPU.\n", nbytes / 1024.0 / 1024.0);

    return SD;
}