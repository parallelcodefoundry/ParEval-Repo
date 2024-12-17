#include "SimpleMOC-kernel_header.h"

extern "C" void setup_kernel(offload_target offload, int num_streams) {
  #pragma omp target teams distribute parallel for offloading(num_streams)
  for (int i = 0; i < num_streams; i++) {
    curandState state;
    curand_init(1234, i, 0, &state);
  }
}

// Initialize global flux states to random numbers on device
// Slow, poor use of GPU, but fine since it's just initialization code
extern "C" void init_flux_states(offload_target offload, int N_flux_states, Input input, curandState * state) {
  #pragma omp target teams distribute parallel for offloading(N_flux_states)
  for (int i = 0; i < N_flux_states; i++) {
    float flux_state;
    if (threadIdx.x == 0) {
      flux_state = curand_uniform(state);
    } else {
      // use a thread-local variable to avoid shared memory access
      __shared__ float local_flux_state;
      local_flux_state = curand_uniform(state);
      flux_state = local_flux_state;
    }
    *(float *)offload[i] = flux_state;
  }
}

// Gets I from user and sets defaults
extern "C" Input set_default_input(void) {
  // ... (no changes needed here)
}

// Returns a memory estimate (in MB) for the program's primary data structures
double mem_estimate(Input input) {
  // ... (no changes needed here)
}