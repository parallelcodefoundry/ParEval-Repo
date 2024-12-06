#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1000000

// Define the device memory layout for the grid
__global__ void init_device_memory() {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < N) {
        double* d_values = (double*)cudaMalloc(sizeof(double), N);
        cudaMemset(d_values, 0.0, sizeof(double) * N);
    }
}

void grid_init() {
    // Initialize OpenMP threads
    omp_set_num_threads(128);

    // Offload the device memory initialization to the GPU
    init_device_memory<<<256, 1024>>>(N);

    // Wait for the kernel to finish
    cudaDeviceSynchronize();

    // Free the device memory
    cudaFree(d_values);
}

int main() {
    grid_init();
    return 0;
}