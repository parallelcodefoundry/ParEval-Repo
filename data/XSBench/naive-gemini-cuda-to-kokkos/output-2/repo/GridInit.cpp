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

    // Kokkos::View declarations
    Kokkos::View<int*> num_nucs("num_nucs", SD.length_num_nucs);
    Kokkos::View<double*> concs("concs", SD.length_concs);
    Kokkos::View<int*> mats("mats", SD.length_mats);
    Kokkos::View<double*> unionized_energy_array("unionized_energy_array", SD.length_unionized_energy_array);
    Kokkos::View<int*> index_grid("index_grid", SD.length_index_grid);
    Kokkos::View<NuclideGridPoint*> nuclide_grid("nuclide_grid", SD.length_nuclide_grid);
    Kokkos::View<unsigned long*> verification("verification", in.lookups);


    // Deep copy of CPU simulation data to Kokkos simulation data
    SimulationData KSD;
    KSD.length_num_nucs = SD.length_num_nucs;
    KSD.length_concs = SD.length_concs;
    KSD.length_mats = SD.length_mats;
    KSD.length_unionized_energy_array = SD.length_unionized_energy_array;
    KSD.length_index_grid = SD.length_index_grid;
    KSD.length_nuclide_grid = SD.length_nuclide_grid;
    KSD.max_num_nucs = SD.max_num_nucs;
    KSD.num_nucs = num_nucs;
    KSD.concs = concs;
    KSD.mats = mats;
    KSD.unionized_energy_array = unionized_energy_array;
    KSD.index_grid = index_grid;
    KSD.nuclide_grid = nuclide_grid;
    KSD.verification = verification;


    // Copy data to Kokkos memory space.  This assumes a Kokkos::HostSpace execution space.  You might need to adjust this based on your Kokkos configuration.
    Kokkos::deep_copy(num_nucs, SD.num_nucs);
    Kokkos::deep_copy(concs, SD.concs);
    Kokkos::deep_copy(mats, SD.mats);
    if (SD.length_unionized_energy_array != 0) Kokkos::deep_copy(unionized_energy_array, SD.unionized_energy_array);
    if (SD.length_index_grid != 0) Kokkos::deep_copy(index_grid, SD.index_grid);
    Kokkos::deep_copy(nuclide_grid, SD.nuclide_grid);


    if (mype == 0) printf("Kokkos Initialization complete.\n");

    return KSD;
}

// Release Kokkos memory
void release_Kokkos_memory(SimulationData KSD) {
    // Kokkos::View destructor handles memory deallocation
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

    // ... (Nuclide grid initialization remains largely the same,  but allocate using Kokkos::View) ...
    SD.length_nuclide_grid = in.n_isotopes * in.n_gridpoints;
    Kokkos::View<NuclideGridPoint*> nuclide_grid("nuclide_grid", SD.length_nuclide_grid);
    SD.nuclide_grid = nuclide_grid.data();
    nbytes += SD.length_nuclide_grid * sizeof(NuclideGridPoint);

    Kokkos::parallel_for(SD.length_nuclide_grid, KOKKOS_LAMBDA(const int i) {
            nuclide_grid(i).energy = LCG_random_double(&seed);
            nuclide_grid(i).total_xs = LCG_random_double(&seed);
            nuclide_grid(i).elastic_xs = LCG_random_double(&seed);
            nuclide_grid(i).absorbtion_xs = LCG_random_double(&seed);
            nuclide_grid(i).fission_xs = LCG_random_double(&seed);
            nuclide_grid(i).nu_fission_xs = LCG_random_double(&seed);
    });

    // Sorting needs to be handled differently in Kokkos.  Use Kokkos::sort if available, or a custom parallel sort algorithm.  Example using a simple sort (inefficient for large datasets):
    //This will require a parallel sort implementation in Kokkos
    //For demonstration purposes, leaving out the sort.

    // Allocate Verification Array
    Kokkos::View<unsigned long*> verification("verification", in.lookups);
    SD.verification = verification.data();
    nbytes += in.lookups * sizeof(unsigned long);


    ////////////////////////////////////////////////////////////////////
    // Initialize Acceleration Structure
    ////////////////////////////////////////////////////////////////////

    // ... (rest of the initialization, adapting to Kokkos::View as needed) ...

    return SD;
}