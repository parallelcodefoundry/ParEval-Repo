
#include <iostream>
#include <omp.h>
#include "init.hpp"
#include "kernel.hpp"

int main() {
    // Set default input parameters
    Input I = set_default_input();

    // Memory allocation for sources and arrays
    Source *sources = initialize_sources(I);
    Source_Arrays SA;
    SA.fine_source_arr = (float *)malloc(I.source_3D_regions * I.fine_axial_intervals * I.egroups * sizeof(float));
    SA.fine_flux_arr = (float *)malloc(I.source_3D_regions * I.fine_axial_intervals * I.egroups * sizeof(float));
    SA.sigT_arr = (float *)malloc(I.source_3D_regions * I.egroups * sizeof(float));

    // Initialize RNG states
    curandState *RNG_states = (curandState *)malloc(I.streams * sizeof(curandState));
    setup_kernel(RNG_states, I);

    // Initialize flux states
    float *flux_states = (float *)malloc(I.segments * sizeof(float));
    init_flux_states(flux_states, I.segments, I, RNG_states);

    // Run the simulation kernel
    run_kernel(I, sources, SA, table, RNG_states, flux_states, I.segments);

    // Free allocated memory
    free(sources);
    free(SA.fine_source_arr);
    free(SA.fine_flux_arr);
    free(SA.sigT_arr);
    free(RNG_states);
    free(flux_states);

    std::cout << "Simulation completed successfully." << std::endl;
    return 0;
}