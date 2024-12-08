#include <stdio.h>
#include <stdlib.h>
#include "XSbench_shared_header.h"

extern "__device__" void DeviceInitialization(int grid_type, int hash_bins);

extern "__host__ __default__" void HostDefaultInitialization() {
    // Do nothing for host default initialization
}

extern "__host__ __parallel__" void HostParallelInitialization(Inputs in) {
    printf("Host parallel initialization started\n");
    // Initialize other variables as needed...
    printf("Host parallel initialization finished\n");
}

extern "__global__" void GridInitialization(int grid_type, int hash_bins) {
    // Device initialization
    DeviceInitialization(grid_type, hash_bins);
}

int main() {
    Inputs in;
    // Parse command-line arguments and initialize inputs structure...

    // Host initialization (before parallelization)
    HostDefaultInitialization();

    #pragma omp offload target(mic:MIC) copyin(in)
    {
        // Offloaded initialization
        GridInitialization(0, 10); // grid_type=0, hash_bins=10
    }

    printf("Device initialization finished\n");

    return EXIT_SUCCESS;
}