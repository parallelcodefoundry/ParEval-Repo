#include "SimpleMOC-kernel_header.hpp"

// Kokkos kernels require a separate function for the kernel launch
// and the actual kernel code.  This is unlike CUDA where the kernel
// launch and kernel code are combined.

// Kokkos::parallel_for does not have a direct analogue to CUDA's
// <<<blocks, threads>>> syntax. The execution policy determines
// how the parallel_for is executed (e.g., on the CPU or GPU).
// Kokkos::RangePolicy is used for simpler parallel_for loops over
// a range of indices.  For more complex scenarios Kokkos offers
// other policies (e.g., Kokkos::TeamPolicy for team-based parallelism).

// Kokkos::parallel_for requires a functor. The functor is a class
// that overloads the operator(). This operator is called for each
// element in the range specified by the execution policy.
struct SetupKernelFunctor {
  Kokkos::View<curandState*,Kokkos::Device> state;
  Input I;

  SetupKernelFunctor(Kokkos::View<curandState*,Kokkos::Device> state_, Input I_) : state(state_), I(I_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(int threadId) const {
    if (threadId >= I.streams) return;
    curand_init(1234, threadId, 0, &state[threadId]);
  }
};

void setup_kernel(Kokkos::View<curandState*,Kokkos::Device> state, Input I) {
  Kokkos::parallel_for(Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(0, I.streams), SetupKernelFunctor(state, I));
}


struct InitFluxStatesFunctor {
  Kokkos::View<float*, Kokkos::Device> flux_states;
  int N_flux_states;
  Input I;
  Kokkos::View<curandState*,Kokkos::Device> state;

  InitFluxStatesFunctor(Kokkos::View<float*, Kokkos::Device> flux_states_, int N_flux_states_, Input I_, Kokkos::View<curandState*,Kokkos::Device> state_) : 
    flux_states(flux_states_), N_flux_states(N_flux_states_), I(I_), state(state_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(int blockId) const {
    if (blockId >= N_flux_states) return;

    curandState* localState = &state[blockId % I.streams];

    for (int i = 0; i < I.egroups; ++i) {
      flux_states[blockId * I.egroups + i] = curand_uniform(localState);
    }
  }
};

void init_flux_states(Kokkos::View<float*, Kokkos::Device> flux_states, int N_flux_states, Input I, Kokkos::View<curandState*,Kokkos::Device> state) {
  Kokkos::parallel_for(Kokkos::RangePolicy<Kokkos::DefaultExecutionSpace>(0, N_flux_states), InitFluxStatesFunctor(flux_states, N_flux_states, I, state));
}


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
  // Allocate & Copy Fine Source Data
  long N_fine = I.source_3D_regions * I.fine_axial_intervals * I.egroups;
  SA_d->fine_source_arr = Kokkos::View<float*>::allocate(N_fine); //Allocate Kokkos View
  Kokkos::deep_copy(SA_d->fine_source_arr, Kokkos::View<const float*>(SA_h->fine_source_arr,N_fine)); //Copy to Kokkos view


  // Allocate & Copy Fine Flux Data
  SA_d->fine_flux_arr = Kokkos::View<float*>::allocate(N_fine);
  Kokkos::deep_copy(SA_d->fine_flux_arr, Kokkos::View<const float*>(SA_h->fine_flux_arr,N_fine));


  // Allocate & Copy SigT Data
  long N_sigT = I.source_3D_regions * I.egroups;
  SA_d->sigT_arr = Kokkos::View<float*>::allocate(N_sigT);
  Kokkos::deep_copy(SA_d->sigT_arr, Kokkos::View<const float*>(SA_h->sigT_arr, N_sigT));


  // Allocate & Copy Source Array Data
  SA_d->sources = Kokkos::View<Source*>::allocate(I.source_3D_regions);
  Kokkos::deep_copy(SA_d->sources, Kokkos::View<const Source*>(sources_h, I.source_3D_regions));

  return SA_d->sources; //return the kokkos view
}

Table buildExponentialTable(void) {
  // define table
  Table table;

  //float precision = 0.01;
  float maxVal = 10.0;

  // compute number of arry values
  //int N = (int) ( maxVal * sqrt(1.0 / ( 8.0 * precision * 0.01 ) ) );
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

void __cudaCheckError(const char* file, const int line) {
  // Kokkos handles error checking differently.  This function would
  // likely be replaced by Kokkos error checking mechanisms.
  //For example using KOKKOS_IF_ERROR  macro or exceptions
}