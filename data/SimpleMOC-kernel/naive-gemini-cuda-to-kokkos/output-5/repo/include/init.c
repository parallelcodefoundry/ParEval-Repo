#include "SimpleMOC-kernel_header.hpp"

// Kokkos kernels require a separate function for the execution space
// and a functor struct that defines the kernel operations

// Functor for setup_kernel
struct SetupKernelFunctor {
    Kokkos::View<curandState*, Kokkos::DeviceSpace> state;
    Input I;

    KOKKOS_INLINE_FUNCTION
    void operator()(const int i) const {
        if (i >= I.streams) return;
        curand_init(1234, i, 0, &state(i));
    }
};

// Function to launch setup_kernel using Kokkos
void setup_kernel_kokkos(Kokkos::View<curandState*, Kokkos::DeviceSpace>& state, Input I) {
    Kokkos::parallel_for(I.streams, SetupKernelFunctor{state, I});
}


// Functor for init_flux_states
struct InitFluxStatesFunctor {
    Kokkos::View<float*, Kokkos::DeviceSpace> flux_states;
    int N_flux_states;
    Input I;
    Kokkos::View<curandState*, Kokkos::DeviceSpace> state;

    KOKKOS_INLINE_FUNCTION
    void operator()(const int i) const {
        if (i >= N_flux_states) return;

        // Assign RNG state.  Need to handle potential out-of-bounds access carefully
        int stream_id = i % I.streams;
        curandState* localState = &state(stream_id);


        for (int j = 0; j < I.egroups; ++j) {
            flux_states(i * I.egroups + j) = curand_uniform(localState);
        }
    }
};

// Function to launch init_flux_states using Kokkos
void init_flux_states_kokkos(Kokkos::View<float*, Kokkos::DeviceSpace>& flux_states, int N_flux_states, Input I, Kokkos::View<curandState*, Kokkos::DeviceSpace>& state) {
    Kokkos::parallel_for(N_flux_states, InitFluxStatesFunctor{flux_states, N_flux_states, I, state});
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
double mem_estimate(Input I) {
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

Source* initialize_sources(Input I, Source_Arrays* SA) {
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

Source* initialize_device_sources(Input I, Source_Arrays* SA_h, Source_Arrays* SA_d, Source* sources_h) {
    // Allocate & Copy Fine Source Data using Kokkos
    long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
    SA_d->fine_source_arr = Kokkos::create_mirror_view(Kokkos::View<float*, Kokkos::DeviceSpace>(SA_h->fine_source_arr, N_fine));
    Kokkos::deep_copy(SA_d->fine_source_arr, SA_h->fine_source_arr);

    // Allocate & Copy Fine Flux Data using Kokkos
    SA_d->fine_flux_arr = Kokkos::create_mirror_view(Kokkos::View<float*, Kokkos::DeviceSpace>(SA_h->fine_flux_arr, N_fine));
    Kokkos::deep_copy(SA_d->fine_flux_arr, SA_h->fine_flux_arr);


    // Allocate & Copy SigT Data using Kokkos
    long N_sigT = I.source_3D_regions * I.egroups;
    SA_d->sigT_arr = Kokkos::create_mirror_view(Kokkos::View<float*, Kokkos::DeviceSpace>(SA_h->sigT_arr, N_sigT));
    Kokkos::deep_copy(SA_d->sigT_arr, SA_h->sigT_arr);

    // Allocate & Copy Source Array Data using Kokkos
    SA_d->sources = Kokkos::create_mirror_view(Kokkos::View<Source*, Kokkos::DeviceSpace>(sources_h, I.source_3D_regions));
    Kokkos::deep_copy(SA_d->sources, sources_h);

    return SA_d->sources;
}


// Builds a table of exponential values for linear interpolation
Table buildExponentialTable(void) {
    // define table
    Table table;

    //float precision = 0.01;
    float maxVal = 10.0;

    // compute number of array values
    //int N = (int)(maxVal * sqrt(1.0 / (8.0 * precision * 0.01)));
    int N = 353;

    // compute spacing
    float dx = maxVal / (float)N;

    // store linear segment information (slope and y-intercept)
    for (int n = 0; n < N; n++) {
        // compute slope and y-intercept for (1 - exp(-x))
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