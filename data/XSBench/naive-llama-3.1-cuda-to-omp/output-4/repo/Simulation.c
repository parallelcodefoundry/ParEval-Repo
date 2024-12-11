#include "XSbench_header.cuh"
#pragma offload target(mic:arch=avx512)
unsigned long long run_event_based_simulation_baseline(Inputs in, SimulationData SD, int mype, Profile* profile) {
    double start = get_time();
    // Move Data to GPU
    SimulationData GSD = move_simulation_data_to_device(in, mype, SD);
    profile->host_to_device_time = get_time() - start;

    ////////////////////////////////////////////////////////////////////////////////
    // Configure & Launch Simulation Kernel
    ////////////////////////////////////////////////////////////////////////////////
    if (mype == 0) printf("Running baseline event-based simulation...\n");

    int nthreads = 256;
    int nblocks = ceil((double)in.lookups / (double)nthreads);

#pragma omp parallel for num_threads(nthreads) offload(target: GSD)
    for (int i = 0; i < in.num_iterations + in.num_warmups; i++) {
        if (i == in.num_warmups) {
            gpuErrchk(cudaDeviceSynchronize());
            start = get_time();
        }
        xs_lookup_kernel_baseline<<<nblocks, nthreads>>>(in, GSD);
    }

#pragma omp task offload(target: SD)
    gpuErrchk(cudaPeekAtLastError());
    gpuErrchk(cudaDeviceSynchronize());

    ////////////////////////////////////////////////////////////////////////////////
    // Reduce Verification Results
    ////////////////////////////////////////////////////////////////////////////////

    if (mype == 0) printf("Reducing verification results...\n");
    start = get_time();
#pragma omp parallel for num_threads(nthreads) offload(target: SD)
    gpuErrchk(cudaMemcpy(SD.verification, GSD.verification, in.lookups * sizeof(unsigned long), cudaMemcpyDeviceToHost));
    profile->device_to_host_time = get_time() - start;

    unsigned long verification_scalar = 0;
    for (int i = 0; i < in.lookups; i++)
        verification_scalar += SD.verification[i];

    release_device_memory(GSD);

    return verification_scalar;
}

// Rest of the code remains the same