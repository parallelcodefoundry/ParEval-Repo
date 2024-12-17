#include "XSbench_header.cuh"
#include <omp.h>

unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile)
{
    double start = get_time();
    // Move Data to GPU
    SimulationData GSD = move_simulation_data_to_device(in, mype, SD);
    profile->host_to_device_time = get_time() - start;

    ////////////////////////////////////////////////////////////////////////////////
    // Configure & Launch Simulation Kernel
    ////////////////////////////////////////////////////////////////////////////////
    if(mype == 0) printf("Running baseline event-based simulation...\n");

    int nthreads = 256;
    int nblocks = ceil( (double) in.lookups / (double) nthreads);

    #pragma omp target teams distribute parallel for num_teams(nblocks) if(mype==0) \
        map(to:in, in.lookups)
    {
        for(int i = 0; i < in.lookups; i++)
            xs_lookup_kernel_baseline<<<nblocks, nthreads>>>(in, GSD);
    }

    gpuErrchk( cudaPeekAtLastError() );
    gpuErrchk( cudaDeviceSynchronize() );

    profile->kernel_time = get_time() - start;

    ////////////////////////////////////////////////////////////////////////////////
    // Reduce Verification Results
    ////////////////////////////////////////////////////////////////////////////////

    if(mype == 0) printf("Reducing verification results...\n");
    start = get_time();
    gpuErrchk(cudaMemcpy(SD.verification, GSD.verification, in.lookups * sizeof(unsigned long), cudaMemcpyDeviceToHost) );
    profile->device_to_host_time = get_time() - start;

    unsigned long verification_scalar = 0;
    for( int i =0; i < in.lookups; i++ )
            verification_scalar += SD.verification[i];

    release_device_memory(GSD);

    return verification_scalar;
}

// Rest of the code remains the same...