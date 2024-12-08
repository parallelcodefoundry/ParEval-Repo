// Modified Materials.cu for OpenMP-offload execution model
#include "XSbench_shared_header.h"
#include <omp.h>

#define NTHREADS 8 // number of threads to use in parallel regions
#define BLOCK_SIZE 256

// Function to calculate the number of particles per thread
int get_particles_per_thread(int num_particles) {
    return (num_particles + (NTHREADS - 1)) / NTHREADS;
}

// Kernel for calculating nuclide lookup time using unionized grid
__global__ void unionized_grid_kernel(float *lookup_times, float *HM, int n_isotopes, int n_gridpoints, int particles_per_thread) {
    // Initialize thread indices
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int idy = blockIdx.y * blockDim.y + threadIdx.y;

    if (idx >= particles_per_thread || idy >= NTHREADS) return;

    // Perform nuclide lookup for each particle in the block
    float lookup_time = 0.0f;
    for (int i = 0; i < n_isotopes; ++i) {
        int grid_index = HM[i * n_gridpoints + idx];
        if (grid_index != -1) {
            lookup_time += lookup_times[grid_index];
        }
    }

    // Update the global memory with the calculated lookup time
    atomicAdd(lookup_times, lookup_time);
}

// Kernel for calculating nuclide lookup time using nuclide grid
__global__ void nuclide_grid_kernel(float *lookup_times, float *HM, int n_isotopes, int n_gridpoints, int particles_per_thread) {
    // Initialize thread indices
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int idy = blockIdx.y * blockDim.y + threadIdx.y;

    if (idx >= particles_per_thread || idy >= NTHREADS) return;

    // Perform nuclide lookup for each particle in the block
    float lookup_time = 0.0f;
    for (int i = 0; i < n_isotopes; ++i) {
        int grid_index = HM[i * n_gridpoints + idx];
        if (grid_index != -1) {
            lookup_time += lookup_times[grid_index];
        }
    }

    // Update the global memory with the calculated lookup time
    atomicAdd(lookup_times, lookup_time);
}

// Kernel for calculating nuclide lookup time using hash grid
__global__ void hash_grid_kernel(float *lookup_times, float *HM, int n_isotopes, int n_hash_bins, int particles_per_thread) {
    // Initialize thread indices
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int idy = blockIdx.y * blockDim.y + threadIdx.y;

    if (idx >= particles_per_thread || idy >= NTHREADS) return;

    // Perform nuclide lookup for each particle in the block
    float lookup_time = 0.0f;
    for (int i = 0; i < n_isotopes; ++i) {
        int hash_index = HM[i * n_hash_bins + idx];
        if (hash_index != -1) {
            lookup_time += lookup_times[hash_index];
        }
    }

    // Update the global memory with the calculated lookup time
    atomicAdd(lookup_times, lookup_time);
}

// Main function for calculating nuclide lookup times
int main() {
    // Initialize OpenMP parallel regions
#pragma omp parallel num_threads(NTHREADS)
{
    // Get the number of particles per thread
    int particles_per_thread = get_particles_per_thread(in.particles);

    // Set block dimensions for each kernel
    dim3 blockSize(BLOCK_SIZE, BLOCK_SIZE);
    dim3 gridSize((particles_per_thread + (blockSize.x - 1)) / blockSize.x,
                  (NTHREADS + (blockSize.y - 1)) / blockSize.y);

    // Allocate memory on the device for lookup times and HM arrays
    float *lookup_times;
    float *HM;

    // Initialize lookup times array with zeros
    cudaMalloc(&lookup_times, in.n_isotopes * sizeof(float));
    cudaMemset(lookup_times, 0, in.n_isotopes * sizeof(float));

    // Allocate memory on the device for HM array
    cudaMalloc(&HM, in.n_isotopes * in.n_gridpoints * sizeof(float));
    memset(HM, -1, in.n_isotopes * in.n_gridpoints * sizeof(float));

    // Copy HM data to the device
    cudaMemcpy(HM, in.HM, in.n_isotopes * in.n_gridpoints * sizeof(float), cudaMemcpyHostToDevice);

    // Perform nuclide lookup using unionized grid kernel
#pragma omp master
{
        if (in.grid_type == 0) {
            unionized_grid_kernel<<<gridSize, blockSize>>>(lookup_times, HM, in.n_isotopes, in.n_gridpoints, particles_per_thread);
        }
        else if (in.grid_type == 1) {
            nuclide_grid_kernel<<<gridSize, blockSize>>>(lookup_times, HM, in.n_isotopes, in.n_gridpoints, particles_per_thread);
        }
        else if (in.grid_type == 2) {
            hash_grid_kernel<<<gridSize, blockSize>>>(lookup_times, HM, in.n_isotopes, in.n_hash_bins, particles_per_thread);
        }

        // Synchronize threads
#pragma omp barrier

        // Calculate the final lookup time by summing up the thread-local results
        float final_lookup_time = 0.0f;
        for (int i = 0; i < NTHREADS; ++i) {
            final_lookup_time += atomicAdd(lookup_times, 0);
        }

        // Copy the final lookup time to the host memory
        cudaMemcpy(&in.lookup_time, &final_lookup_time, sizeof(float), cudaMemcpyDeviceToHost);

        // Free device memory
        cudaFree(lookup_times);
        cudaFree(HM);
    }
}
}