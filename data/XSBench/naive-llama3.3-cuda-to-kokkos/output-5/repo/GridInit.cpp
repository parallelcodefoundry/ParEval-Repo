#include "XSbench_header.hpp"

// Moves all required data structures to the GPU's memory space
SimulationData move_simulation_data_to_device(Inputs in, int mype, SimulationData SD) {
    if (mype == 0) printf("Allocating and moving simulation data to GPU memory space...\n");

    // SUMMARY: Simulation Data Structure Manifest for "SD" Object
    // Here we list all heap arrays (and lengths) in SD that would need to be
    // offloaded manually if using an accelerator with a seperate memory space
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
    // 

    // Shallow copy of CPU simulation data to GPU simulation data
    SimulationData GSD = SD;

    // Move data to GPU memory space
    Kokkos::deep_copy(GSD.num_nucs, SD.num_nucs);
    Kokkos::deep_copy(GSD.concs, SD.concs);
    Kokkos::deep_copy(GSD.mats, SD.mats);
    if (SD.length_unionized_energy_array > 0) Kokkos::deep_copy(GSD.unionized_energy_array, SD.unionized_energy_array);
    Kokkos::deep_copy(GSD.nuclide_grid, SD.nuclide_grid);
    if (SD.length_index_grid > 0) Kokkos::deep_copy(GSD.index_grid, SD.index_grid);

    // Allocate verification array on device. This structure is not needed on CPU, so we don't
    // have to copy anything over.
    GSD.verification = Kokkos::View<unsigned long*>("verification", in.lookups);

    // Synchronize
    Kokkos::fence();

    if (mype == 0) printf("GPU Intialization complete.\n");

    return GSD;
}

// Release device memory
void release_device_memory(SimulationData GSD) {
    GSD.num_nucs = nullptr;
    GSD.concs = nullptr;
    GSD.mats = nullptr;
    GSD.unionized_energy_array = nullptr;
    GSD.nuclide_grid = nullptr;
    GSD.index_grid = nullptr;
    GSD.verification = nullptr;
}

void release_memory(SimulationData SD) {
    SD.num_nucs = nullptr;
    SD.concs = nullptr;
    SD.mats = nullptr;
    SD.unionized_energy_array = nullptr;
    SD.nuclide_grid = nullptr;
    SD.index_grid = nullptr;
    SD.verification = nullptr;
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

    // First, we need to initialize our nuclide grid. This comes in the form
    // of a flattened 2D array that hold all the information we need to define
    // the cross sections for all isotopes in the simulation.
    // The grid is composed of "NuclideGridPoint" structures, which hold the
    // energy level of the grid point and all associated XS data at that level.
    // An array of structures (AOS) is used instead of
    // a structure of arrays, as the grid points themselves are accessed in
    // a random order, but all cross section interaction channels and the
    // energy level are read whenever the gridpoint is accessed, meaning the
    // AOS is more cache efficient.

    // Initialize Nuclide Grid
    SD.length_nuclide_grid = in.n_isotopes * in.n_gridpoints;
    SD.nuclide_grid = Kokkos::View<NuclideGridPoint*>("nuclide_grid", SD.length_nuclide_grid);
    nbytes += SD.length_nuclide_grid * sizeof(NuclideGridPoint);
    Kokkos::parallel_for(Kokkos::RangePolicy<>(0, SD.length_nuclide_grid), KOKKOS_LAMBDA(const int& i) {
        SD.nuclide_grid(i).energy = LCG_random_double(&seed);
        SD.nuclide_grid(i).total_xs = LCG_random_double(&seed);
        SD.nuclide_grid(i).elastic_xs = LCG_random_double(&seed);
        SD.nuclide_grid(i).absorbtion_xs = LCG_random_double(&seed);
        SD.nuclide_grid(i).fission_xs = LCG_random_double(&seed);
        SD.nuclide_grid(i).nu_fission_xs = LCG_random_double(&seed);
    });
    Kokkos::fence();

    // Sort so that each nuclide has data stored in ascending energy order.
    for (int i = 0; i < in.n_isotopes; i++) {
        Kokkos::View<NuclideGridPoint*> nuclide_grid_view = Kokkos::subview(SD.nuclide_grid, Kokkos::make_pair(i * in.n_gridpoints, in.n_gridpoints));
        std::sort(nuclide_grid_view.data(), nuclide_grid_view.data() + in.n_gridpoints, NGP_compare);
    }

    // Allocate Verification Array
    size_t sz = in.lookups * sizeof(unsigned long);
    SD.verification = Kokkos::View<unsigned long*>("verification", in.lookups);
    nbytes += sz;

    ////////////////////////////////////////////////////////////////////
    // Initialize Acceleration Structure
    ////////////////////////////////////////////////////////////////////

    if (in.grid_type == NUCLIDE) {
        SD.length_unionized_energy_array = 0;
        SD.length_index_grid = 0;
    }

    if (in.grid_type == UNIONIZED) {
        if (mype == 0) printf("Intializing unionized grid...\n");

        // Allocate space to hold the union of all nuclide energy data
        SD.length_unionized_energy_array = in.n_isotopes * in.n_gridpoints;
        SD.unionized_energy_array = Kokkos::View<double*>("unionized_energy_array", SD.length_unionized_energy_array);
        nbytes += SD.length_unionized_energy_array * sizeof(double);

        // Copy energy data over from the nuclide energy grid
        Kokkos::parallel_for(Kokkos::RangePolicy<>(0, SD.length_unionized_energy_array), KOKKOS_LAMBDA(const int& i) {
            SD.unionized_energy_array(i) = SD.nuclide_grid(i).energy;
        });
        Kokkos::fence();

        // Sort unionized energy array
        std::sort(SD.unionized_energy_array.data(), SD.unionized_energy_array.data() + SD.length_unionized_energy_array);

        // Allocate space to hold the acceleration grid indices
        SD.length_index_grid = SD.length_unionized_energy_array * in.n_isotopes;
        SD.index_grid = Kokkos::View<int*>("index_grid", SD.length_index_grid);
        nbytes += SD.length_index_grid * sizeof(int);

        // Generates the double indexing grid
        Kokkos::parallel_for(Kokkos::RangePolicy<>(0, SD.length_unionized_energy_array), KOKKOS_LAMBDA(const int& e) {
            double unionized_energy = SD.unionized_energy_array(e);
            for (long i = 0; i < in.n_isotopes; i++) {
                if (unionized_energy < SD.nuclide_grid[i * in.n_gridpoints + 1].energy)
                    SD.index_grid(e * in.n_isotopes + i) = 0;
                else if (SD.nuclide_grid[i * in.n_gridpoints + in.n_gridpoints - 1].energy < unionized_energy)
                    SD.index_grid(e * in.n_isotopes + i) = in.n_gridpoints - 1;
                else {
                    int idx = grid_search_nuclide(in.n_gridpoints, unionized_energy, SD.nuclide_grid + i * in.n_gridpoints, 0, in.n_gridpoints - 1);
                    SD.index_grid(e * in.n_isotopes + i) = idx;
                }
            }
        });
        Kokkos::fence();
    }

    if (in.grid_type == HASH) {
        if (mype == 0) printf("Intializing hash grid...\n");
        SD.length_unionized_energy_array = 0;
        SD.length_index_grid = in.hash_bins * in.n_isotopes;
        SD.index_grid = Kokkos::View<int*>("index_grid", SD.length_index_grid);
        nbytes += SD.length_index_grid * sizeof(int);

        double du = 1.0 / in.hash_bins;

        // For each energy level in the hash table
        Kokkos::parallel_for(Kokkos::RangePolicy<>(0, in.hash_bins), KOKKOS_LAMBDA(const int& e) {
            double energy = e * du;

            // We need to determine the bounding energy levels for all isotopes
            for (long i = 0; i < in.n_isotopes; i++) {
                SD.index_grid(e * in.n_isotopes + i) = grid_search_nuclide(in.n_gridpoints, energy, SD.nuclide_grid + i * in.n_gridpoints, 0, in.n_gridpoints - 1);
            }
        });
        Kokkos::fence();
    }

    ////////////////////////////////////////////////////////////////////
    // Initialize Materials and Concentrations
    ////////////////////////////////////////////////////////////////////
    if (mype == 0) printf("Intializing material data...\n");

    // Set the number of nuclides in each material
    SD.num_nucs = load_num_nucs(in.n_isotopes);
    SD.length_num_nucs = 12; // There are always 12 materials in XSBench

    // Intialize the flattened 2D grid of material data. The grid holds
    // a list of nuclide indices for each of the 12 material types. The
    // grid is allocated as a full square grid, even though not all
    // materials have the same number of nuclides.
    SD.mats = load_mats(SD.num_nucs, in.n_isotopes, &SD.max_num_nucs);
    SD.length_mats = SD.length_num_nucs * SD.max_num_nucs;

    // Intialize the flattened 2D grid of nuclide concentration data. The grid holds
    // a list of nuclide concentrations for each of the 12 material types. The
    // grid is allocated as a full square grid, even though not all
    // materials have the same number of nuclides.
    SD.concs = load_concs(SD.num_nucs, SD.max_num_nucs);
    SD.length_concs = SD.length_mats;

    if (mype == 0) printf("Intialization complete.\n");

    return SD;
}