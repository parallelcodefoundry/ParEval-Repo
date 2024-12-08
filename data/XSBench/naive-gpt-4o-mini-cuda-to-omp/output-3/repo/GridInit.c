#include "XSbench_header.cuh"
#include <omp.h>

// Moves all required data structures to the GPU's memory space
SimulationData move_simulation_data_to_device(Inputs in, int mype, SimulationData SD)
{
    if (mype == 0) printf("Allocating and moving simulation data to GPU memory space...\n");

    // Shallow copy of CPU simulation data to GPU simulation data
    SimulationData GSD = SD;

    // Move data to GPU memory space
    #pragma omp target data map(to: SD.num_nucs[0:SD.length_num_nucs], SD.concs[0:SD.length_concs], SD.mats[0:SD.length_mats], SD.nuclide_grid[0:SD.length_nuclide_grid]) \
                             map(to: in.lookups) map(from: GSD.num_nucs, GSD.concs, GSD.mats, GSD.nuclide_grid)
    {
        size_t sz;
        size_t total_sz = 0;

        sz = GSD.length_num_nucs * sizeof(int);
        #pragma omp target enter data map(to: GSD.num_nucs[0:sz]) map(from: SD.num_nucs[0:sz])
        total_sz += sz;

        sz = GSD.length_concs * sizeof(double);
        #pragma omp target enter data map(to: GSD.concs[0:sz]) map(from: SD.concs[0:sz])
        total_sz += sz;

        sz = GSD.length_mats * sizeof(int);
        #pragma omp target enter data map(to: GSD.mats[0:sz]) map(from: SD.mats[0:sz])
        total_sz += sz;

        if (SD.length_unionized_energy_array != 0) {
            sz = GSD.length_unionized_energy_array * sizeof(double);
            #pragma omp target enter data map(to: GSD.unionized_energy_array[0:sz]) map(from: SD.unionized_energy_array[0:sz])
            total_sz += sz;
        }

        if (SD.length_index_grid != 0) {
            sz = GSD.length_index_grid * sizeof(int);
            #pragma omp target enter data map(to: GSD.index_grid[0:sz]) map(from: SD.index_grid[0:sz])
            total_sz += sz;
        }

        sz = GSD.length_nuclide_grid * sizeof(NuclideGridPoint);
        #pragma omp target enter data map(to: GSD.nuclide_grid[0:sz]) map(from: SD.nuclide_grid[0:sz])
        total_sz += sz;

        // Allocate verification array on device. This structure is not needed on CPU, so we don't
        // have to copy anything over.
        sz = in.lookups * sizeof(unsigned long);
        #pragma omp target enter data map(to: GSD.verification[0:sz])
        total_sz += sz;
        GSD.length_verification = in.lookups;

        // Synchronize
        #pragma omp target update self(GSD)

        if (mype == 0) printf("GPU Initialization complete. Allocated %.0lf MB of data on GPU.\n", total_sz / 1024.0 / 1024.0);
    }

    return GSD;
}

// Release device memory
void release_device_memory(SimulationData GSD) {
    #pragma omp target exit data map(delete: GSD.num_nucs, GSD.concs, GSD.mats, GSD.unionized_energy_array, GSD.nuclide_grid, GSD.verification)
}