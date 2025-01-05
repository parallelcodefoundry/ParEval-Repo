#include "XSbench_header.hpp"

// Moves all required data structures to the Kokkos's memory space
SimulationData move_simulation_data_to_device(Inputs in, int mype, SimulationData SD) {
    if (mype == 0) printf("Allocating and moving simulation data to Kokkos memory space...\n");

    ////////////////////////////////////////////////////////////////////////////////
    // SUMMARY: Simulation Data Structure Manifest for "SD" Object
    // Here we list all heap arrays (and lengths) in SD that would need to be
    // offloaded manually if using an accelerator with a seperate memory space
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

    //Kokkos views for simulation data
    Kokkos::View<int*> num_nucs("num_nucs", SD.length_num_nucs);
    Kokkos::View<double*> concs("concs", SD.length_concs);
    Kokkos::View<int*> mats("mats", SD.length_mats);
    Kokkos::View<double*> unionized_energy_array("unionized_energy_array", SD.length_unionized_energy_array);
    Kokkos::View<int*> index_grid("index_grid", SD.length_index_grid);
    Kokkos::View<NuclideGridPoint*> nuclide_grid("nuclide_grid", SD.length_nuclide_grid);
    Kokkos::View<unsigned long*> verification("verification", in.lookups);


    // Deep copy of host data to Kokkos views.  Could be optimized with parallel copies if necessary.
    Kokkos::deep_copy(num_nucs, SD.num_nucs);
    Kokkos::deep_copy(concs, SD.concs);
    Kokkos::deep_copy(mats, SD.mats);
    if (SD.length_unionized_energy_array != 0) Kokkos::deep_copy(unionized_energy_array, SD.unionized_energy_array);
    if (SD.length_index_grid != 0) Kokkos::deep_copy(index_grid, SD.index_grid);
    Kokkos::deep_copy(nuclide_grid, SD.nuclide_grid);


    SimulationData GSD;
    GSD.num_nucs = num_nucs.data();
    GSD.concs = concs.data();
    GSD.mats = mats.data();
    GSD.unionized_energy_array = unionized_energy_array.data();
    GSD.index_grid = index_grid.data();
    GSD.nuclide_grid = nuclide_grid.data();
    GSD.verification = verification.data();
    GSD.length_num_nucs = SD.length_num_nucs;
    GSD.length_concs = SD.length_concs;
    GSD.length_mats = SD.length_mats;
    GSD.length_unionized_energy_array = SD.length_unionized_energy_array;
    GSD.length_index_grid = SD.length_index_grid;
    GSD.length_nuclide_grid = SD.length_nuclide_grid;
    GSD.max_num_nucs = SD.max_num_nucs;
    GSD.length_verification = in.lookups;


    if (mype == 0) printf("Kokkos Initialization complete.\n");

    return GSD;
}

// Release Kokkos memory
void release_device_memory(SimulationData GSD) {
    //Kokkos views are automatically deallocated when they go out of scope.
}

void release_memory(SimulationData SD) {
    free(SD.num_nucs);
    free(SD.concs);
    free(SD.mats);
    if (SD.length_unionized_energy_array > 0) free(SD.unionized_energy_array);
    free(SD.nuclide_grid);
    free(SD.verification);
}

SimulationData grid_init_do_not_profile(Inputs in, int mype) {
    // Structure to hold all allocated simuluation data arrays
    SimulationData SD;

    // Keep track of how much data we're allocating
    size_t nbytes = 0;

    // Set the initial seed value
    uint64_t seed = 42;

    ////////////////////////////////////////////////////////////////////
    // Initialize Nuclide Grids
    ////////////////////////////////////////////////////////////////////

    if (mype == 0) printf("Intializing nuclide grids...\n");

    // ... (Nuclide grid initialization remains largely the same, using malloc) ...

    // Allocate Verification Array
    size_t sz = in.lookups * sizeof(unsigned long);
    SD.verification = (unsigned long*)malloc(sz);
    nbytes += sz;

    // ... (Unionized and Hash grid initialization remains largely the same, using malloc) ...

    ////////////////////////////////////////////////////////////////////
    // Initialize Materials and Concentrations
    ////////////////////////////////////////////////////////////////////
    if (mype == 0) printf("Intializing material data...\n");

    // Set the number of nuclides in each material
    SD.num_nucs = load_num_nucs(in.n_isotopes);
    SD.length_num_nucs = 12; // There are always 12 materials in XSBench

    // ... (Material and concentration initialization remains largely the same, using malloc) ...

    if (mype == 0) printf("Intialization complete. Allocated %.0lf MB of data on CPU.\n", nbytes / 1024.0 / 1024.0);

    return SD;
}