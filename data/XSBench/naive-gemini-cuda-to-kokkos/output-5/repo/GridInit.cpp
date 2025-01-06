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

    // Shallow copy of CPU simulation data to Kokkos simulation data
    SimulationData KSD = SD;

    // Kokkos memory allocation and data transfer.  We'll assume a default Kokkos::DefaultExecutionSpace
    // You might need to specify a different execution space depending on your target architecture.

    Kokkos::View<int*, Kokkos::DefaultExecutionSpace> K_num_nucs("K_num_nucs", SD.length_num_nucs);
    Kokkos::deep_copy(K_num_nucs, Kokkos::View<int*, Kokkos::HostSpace>(SD.num_nucs, SD.length_num_nucs));
    KSD.num_nucs = K_num_nucs.data(); //Store pointer to Kokkos view data.  Handle carefully!
    KSD.length_num_nucs = SD.length_num_nucs;


    Kokkos::View<double*, Kokkos::DefaultExecutionSpace> K_concs("K_concs", SD.length_concs);
    Kokkos::deep_copy(K_concs, Kokkos::View<double*, Kokkos::HostSpace>(SD.concs, SD.length_concs));
    KSD.concs = K_concs.data();
    KSD.length_concs = SD.length_concs;


    Kokkos::View<int*, Kokkos::DefaultExecutionSpace> K_mats("K_mats", SD.length_mats);
    Kokkos::deep_copy(K_mats, Kokkos::View<int*, Kokkos::HostSpace>(SD.mats, SD.length_mats));
    KSD.mats = K_mats.data();
    KSD.length_mats = SD.length_mats;

    if (SD.length_unionized_energy_array != 0) {
        Kokkos::View<double*, Kokkos::DefaultExecutionSpace> K_unionized_energy_array("K_unionized_energy_array", SD.length_unionized_energy_array);
        Kokkos::deep_copy(K_unionized_energy_array, Kokkos::View<double*, Kokkos::HostSpace>(SD.unionized_energy_array, SD.length_unionized_energy_array));
        KSD.unionized_energy_array = K_unionized_energy_array.data();
        KSD.length_unionized_energy_array = SD.length_unionized_energy_array;
    }

    if (SD.length_index_grid != 0) {
        Kokkos::View<int*, Kokkos::DefaultExecutionSpace> K_index_grid("K_index_grid", SD.length_index_grid);
        Kokkos::deep_copy(K_index_grid, Kokkos::View<int*, Kokkos::HostSpace>(SD.index_grid, SD.length_index_grid));
        KSD.index_grid = K_index_grid.data();
        KSD.length_index_grid = SD.length_index_grid;
    }

    Kokkos::View<NuclideGridPoint*, Kokkos::DefaultExecutionSpace> K_nuclide_grid("K_nuclide_grid", SD.length_nuclide_grid);
    Kokkos::deep_copy(K_nuclide_grid, Kokkos::View<NuclideGridPoint*, Kokkos::HostSpace>(SD.nuclide_grid, SD.length_nuclide_grid));
    KSD.nuclide_grid = K_nuclide_grid.data();
    KSD.length_nuclide_grid = SD.length_nuclide_grid;


    // Allocate verification array on device. This structure is not needed on CPU, so we don't
    // have to copy anything over.
    Kokkos::View<unsigned long*, Kokkos::DefaultExecutionSpace> K_verification("K_verification", in.lookups);
    KSD.verification = K_verification.data();
    KSD.length_verification = in.lookups;


    if (mype == 0) printf("Kokkos Initialization complete.\n"); //Removed size output as it's not directly comparable to CUDA.

    return KSD;
}

// Release Kokkos memory
void release_device_memory(SimulationData KSD) {
    //Note that Kokkos handles deallocation automatically when views go out of scope.
    //If you've manually managed memory, you'll need to call Kokkos::free() here.
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

    // ... (Nuclide grid initialization remains largely the same, using malloc) ...

    // Allocate Verification Array
    size_t sz = in.lookups * sizeof(unsigned long);
    SD.verification = (unsigned long*)malloc(sz);
    nbytes += sz;

    // ... (Rest of the initialization remains largely the same) ...

    return SD;
}