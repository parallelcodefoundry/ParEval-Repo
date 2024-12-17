#include "XSbench_header.h"

// BASELINE FUNCTIONS
// ...

unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile)
{
    double start = get_time();
    // Move Data to GPU
    SimulationData GSD = move_simulation_data_to_device(in, mype, SD);
    profile->host_to_device_time = get_time() - start;

    // ... (rest of the function remains the same)

    // Launch simulation kernel using OpenMP offload
    #pragma omp target teams distribute parallel for num_threads(256)
    {
        xs_lookup_kernel_baseline<<<nblocks, nthreads>>>(in, GSD);
    }
    gpuErrchk(omp_get_thread_num());
}

// In this kernel, we perform a single lookup with each thread. Threads within a warp
// do not really have any relation to each other, and divergence due to high nuclide count fuel
// material lookups are costly. This kernel constitutes baseline performance.
__global__
void xs_lookup_kernel_baseline(Inputs in, SimulationData GSD )
{
    // The lookup ID. Used to set the seed, and to store the verification value
    const int i = blockIdx.x * blockDim.x + threadIdx.x;

    if( i >= in.lookups )
        return;

    // Set the initial seed value
    uint64_t seed = STARTING_SEED;

    // Forward seed to lookup index (we need 2 samples per lookup)
    seed = fast_forward_LCG(seed, 2*i);

    // ... (rest of the kernel remains the same)

    #pragma omp target device_ptr(GSD.verification)
    {
        GSD.verification[i] = max_idx+1;
    }
}

// ... (rest of the file remains the same)