#include <omp.h>
#include "XSbench_shared_header.h"

__global__ void simulate_kernel(double *x, double *y, int nthreads, long n_isotopes, long n_gridpoints) {
  // Offload kernel to GPU
  __shared__ double grid[256];
  if (threadIdx.x < n_gridpoints) {
    grid[threadIdx.x] = x[n_gridpoints + threadIdx.x];
  }
  __syncthreads();

  int isotope_id = blockIdx.x * blockDim.x + threadIdx.x;
  while (isotope_id < n_isotopes) {
    double y_value = y[isotope_id];
    // Perform lookups and calculations here
    grid[threadIdx.x] += y_value;
    isotope_id += blockDim.x * gridDim.x;
  }
}

int main() {
  Inputs in;
  Profile profile;

  // Initialize inputs
  in.nthreads = omp_get_num_threads();
  in.n_isotopes = n_isotopes;
  in.n_gridpoints = n_gridpoints;
  in.lookups = lookups;
  in.grid_type = grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
  in.hash_bins = hash_bins;
  in.particles = particles;
  in.simulation_method = simulation_method;
  in.binary_mode = binary_mode;
  in.kernel_id = kernel_id;
  in.num_iterations = num_iterations;
  in.num_warmups = num_warmups;

  // Perform OpenMP parallelization
  #pragma omp parallel for shared(in, profile) num_threads(in.nthreads)
  for (int i = 0; i < in.particles; i++) {
    double x[i], y[i];
    simulate_kernel<<<1, 256>>>(x, y, in.nthreads, in.n_isotopes, in.n_gridpoints);
  }

  // Wait for offload to complete
  cudaDeviceSynchronize();

  // Profile kernel execution time
  profile.kernel_time = clock() - start_clock;

  return 0;
}