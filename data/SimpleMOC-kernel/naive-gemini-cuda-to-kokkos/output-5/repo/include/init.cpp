#include "SimpleMOC-kernel_header.hpp"

// Kokkos kernels require a separate function for the kernel launch
// and the kernel execution.
void setup_kernel_kokkos(Kokkos::View<curandState*, Kokkos::HostSpace> state, const Input& I) {
    Kokkos::parallel_for(Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(0, I.streams), 
                         KOKKOS_LAMBDA(int i) {
        curand_init(1234, i, 0, &state[i]);
    });
}


// Initialize global flux states to random numbers on device
// Slow, poor use of GPU, but fine since it's just initialization code
void init_flux_states_kokkos(Kokkos::View<float*, Kokkos::DefaultExecutionSpace> flux_states, int N_flux_states, const Input& I, Kokkos::View<curandState*, Kokkos::DefaultExecutionSpace> state) {
    Kokkos::parallel_for(Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(0, N_flux_states),
                         KOKKOS_LAMBDA(int i) {
        // Assign RNG state (assuming a suitable mapping for stream assignment)
        int stream_id = i % I.streams;  //Simple stream mapping, adjust as needed
        curandState* localState = &state[stream_id];

        for (int j = 0; j < I.egroups; ++j) {
            flux_states(i * I.egroups + j) = curand_uniform(localState);
        }
    });
}


// Gets I from user and sets defaults
Input set_default_input(void) {
    Input I;

    I.source_2D_regions = 5000;
    I.coarse_axial_intervals = 27;
    I.fine_axial_intervals = 5;
    I.decomp_assemblies_ax = 20; // Number of subdomains per assembly axially
    I.segments = 50000000;
    I.egroups = 128;
    I.streams = 10000;
    I.seg_per_thread = 100;

    return I;
}

// Returns a memory estimate (in MB) for the program's primary data structures
double mem_estimate(const Input& I) {
    size_t nbytes = 0;

    // Sources Array
    nbytes += I.source_3D_regions * sizeof(Source);

    // Fine Source Data
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    nbytes += N_fine * sizeof(float);

    // Fine Flux Data
    nbytes += N_fine * sizeof(float);

    // SigT Data
    long N_sigT = I.source_3D_regions * I.egroups;
    nbytes += N_sigT * sizeof(float);

    // Return MB
    return (double)nbytes / 1024.0 / 1024.0;
}

Source* initialize_sources(const Input& I, Source_Arrays* SA) {
    // Source Data Structure Allocation
    Source* sources = (Source*)malloc(I.source_3D_regions * sizeof(Source));

    // Allocate Fine Source Data
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    SA->fine_source_arr = (float*)malloc(N_fine * sizeof(float));
    for (int i = 0; i < I.source_3D_regions; i++)
        sources[i].fine_source_id = i * I.fine_axial_intervals * I.egroups;

    // Allocate Fine Flux Data
    SA->fine_flux_arr = (float*)malloc(N_fine * sizeof(float));
    for (int i = 0; i < I.source_3D_regions; i++)
        sources[i].fine_flux_id = i * I.fine_axial_intervals * I.egroups;

    // Allocate SigT Data
    long N_sigT = I.source_3D_regions * I.egroups;
    SA->sigT_arr = (float*)malloc(N_sigT * sizeof(float));
    for (int i = 0; i < I.source_3D_regions; i++)
        sources[i].sigT_id = i * I.egroups;

    // Initialize fine source and flux to random numbers
    for (long i = 0; i < N_fine; i++) {
        SA->fine_source_arr[i] = (float)rand() / RAND_MAX;
        SA->fine_flux_arr[i] = (float)rand() / RAND_MAX;
    }

    // Initialize SigT Values
    for (int i = 0; i < N_sigT; i++)
        SA->sigT_arr[i] = (float)rand() / RAND_MAX;

    return sources;
}

Source* initialize_device_sources(const Input& I, Source_Arrays* SA_h, Source_Arrays* SA_d, Source* sources_h) {
    //Kokkos Views for device data
    Kokkos::View<float*, Kokkos::DefaultExecutionSpace> d_fine_source("fine_source", I.source_3D_regions * I.fine_axial_intervals * I.egroups);
    Kokkos::View<float*, Kokkos::DefaultExecutionSpace> d_fine_flux("fine_flux", I.source_3D_regions * I.fine_axial_intervals * I.egroups);
    Kokkos::View<float*, Kokkos::DefaultExecutionSpace> d_sigT("sigT", I.source_3D_regions * I.egroups);
    Kokkos::View<Source*, Kokkos::DefaultExecutionSpace> d_sources("sources", I.source_3D_regions);


    //Copy host data to Kokkos views
    Kokkos::deep_copy(d_fine_source, Kokkos::View<float*,Kokkos::HostSpace>(SA_h->fine_source_arr, I.source_3D_regions * I.fine_axial_intervals * I.egroups));
    Kokkos::deep_copy(d_fine_flux, Kokkos::View<float*,Kokkos::HostSpace>(SA_h->fine_flux_arr, I.source_3D_regions * I.fine_axial_intervals * I.egroups));
    Kokkos::deep_copy(d_sigT, Kokkos::View<float*,Kokkos::HostSpace>(SA_h->sigT_arr, I.source_3D_regions * I.egroups));
    Kokkos::deep_copy(d_sources, Kokkos::View<Source*,Kokkos::HostSpace>(sources_h, I.source_3D_regions));

    SA_d->fine_source_arr = d_fine_source.data();
    SA_d->fine_flux_arr = d_fine_flux.data();
    SA_d->sigT_arr = d_sigT.data();
    return d_sources.data();
}

// Builds a table of exponential values for linear interpolation
Table buildExponentialTable(void) {
    // define table
    Table table;

    float maxVal = 10.0;

    int N = 353;

    // compute spacing
    float dx = maxVal / (float)N;

    // store linear segment information (slope and y-intercept)
    for (int n = 0; n < N; n++) {
        // compute slope and y-intercept for ( 1 - exp(-x) )
        float exponential = exp(-n * dx);
        table.values[2 * n] = -exponential;
        table.values[2 * n + 1] = 1 + (n * dx - 1) * exponential;
    }

    // assign data to table
    table.dx = dx;
    table.maxVal = maxVal - table.dx;
    table.N = N;

    return table;
}