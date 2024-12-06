#include <stdio.h>
#include <omp.h>

// ... (rest of the code remains the same)

void main() {
  // Create parallel region for data access and compute operations
  #pragma omp parallel for reduction(+:num_iterations, num_warmups)
  for(int i=0; i<in.num_particles; ++i) {
    int material_index = in.particles[i].material;
    double energy = in.particles[i].energy;

    // Perform binary search
    if (in.binary_mode) {
      // Binary search is offloaded to the GPU
      #pragma omp target(dataflow)
      __global__ void binary_search(double* results, int* indices, int size, long iterations, int num_warmups, double device_to_host_time, double host_to_device_time, Profile profile, Inputs in) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx < size) {
          // Perform binary search
          for (int j=0; j<in.lookups; ++j) {
            long num_isotopes = in.n_isotopes[in.grid_type];
            double* isotopes = results[idx*num_isotopes + j * in.grid_type];
            int* indices_data = indices[idx*num_isotopes + j * in.grid_type];

            // Perform kernel
            if (in.kernel_id == 0) {
              // Kernel for event-based model
              kernel_event_based(data, isotopes, indices_data, num_iterations, num_warmups);
            }
            else if (in.kernel_id == 1) {
              // Kernel for Monte Carlo reactor analysis
              kernel_montecarlo_reactor_analysis(data, isotopes, indices_data, num_iterations, num_warmups);
            }

            // Update profile data
            profile.device_to_host_time += device_to_host_time;
            profile.kernel_time += kernel_time;
          }
        }
      }

      // Initialize variables and launch binary search on GPU
      double* results = (double*)malloc(in.n_gridpoints * in.grid_type * sizeof(double));
      int* indices = (int*)malloc(in.n_gridpoints * in.grid_type * sizeof(int));

      for (int j=0; j<in.lookups; ++j) {
        long num_isotopes = in.n_isotopes[in.grid_type];
        double* isotopes = results + idx * num_isotopes;
        int* indices_data = indices + idx * num_isotopes;

        // Initialize variables
        for (int k=0; k<num_isotopes; ++k) {
          isotopes[k] = data[idx*num_isotopes + j * in.grid_type];
          indices_data[k] = k;
        }
      }

      // Launch binary search on GPU
      int grid_size = pow(2, ceil(log(in.num_particles)/log(2)));
      int block_size = 256;

      #pragma omp target(map(tofrom:results, indices))
      __global__ void binary_search_kernel(int size, long iterations, int num_warmups) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx < size) {
          // Perform binary search
          for (int j=0; j<iterations; ++j) {
            long num_isotopes = in.n_isotopes[in.grid_type];
            double* isotopes = results[idx*num_isotopes + j * in.grid_type];
            int* indices_data = indices[idx*num_isotopes + j * in.grid_type];

            // Perform kernel
            if (in.kernel_id == 0) {
              // Kernel for event-based model
              kernel_event_based(data, isotopes, indices_data);
            }
            else if (in.kernel_id == 1) {
              // Kernel for Monte Carlo reactor analysis
              kernel_montecarlo_reactor_analysis(data, isotopes, indices_data);
            }

            // Update profile data
            host_to_device_time += device_to_host_time;
          }
        }
      }

      // Launch binary search on GPU
      dim3 blocks(grid_size, 1);
      dim3 threads(block_size);

      #pragma omp target(map(from:results, indices))
      __global__ void launch_binary_search_kernel(int size) {
        int idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (idx < size) {
          // Perform binary search
          for (int j=0; j<in.lookups; ++j) {
            long num_isotopes = in.n_isotopes[in.grid_type];
            double* isotopes = results[idx*num_isotopes + j * in.grid_type];
            int* indices_data = indices[idx*num_isotopes + j * in.grid_type];

            // Perform kernel
            if (in.kernel_id == 0) {
              // Kernel for event-based model
              kernel_event_based(data, isotopes, indices_data);
            }
            else if (in.kernel_id == 1) {
              // Kernel for Monte Carlo reactor analysis
              kernel_montecarlo_reactor_analysis(data, isotopes, indices_data);
            }

            // Update profile data
            device_to_host_time += host_to_device_time;
          }
        }
      }

      // Launch binary search on GPU
      launch_binary_search_kernel(in.num_particles);

      // Finalize variables and copy results back to host
      for (int j=0; j<in.lookups; ++j) {
        long num_isotopes = in.n_isotopes[in.grid_type];
        double* isotopes = results + idx * num_isotopes;

        // Copy results back to host
        for (int k=0; k<num_isotopes; ++k) {
          data[idx*num_isotopes + j * in.grid_type] = isotopes[k];
        }
      }

      free(results);
      free(indices);
    }

    // Check if particle is found
    if (results[material_index*in.lookups+energy] == 1.0) {
      // Particle is found, update material index and energy
      in.particles[i].material = material_index;
      in.particles[i].energy = energy;

      // Perform binary search again
      for (int j=0; j<in.lookups; ++j) {
        long num_isotopes = in.n_isotopes[in.grid_type];
        double* isotopes = results + material_index * num_isotopes;
        int* indices_data = indices + material_index * num_isotopes;

        // Perform kernel
        if (in.kernel_id == 0) {
          // Kernel for event-based model
          kernel_event_based(data, isotopes, indices_data);
        }
        else if (in.kernel_id == 1) {
          // Kernel for Monte Carlo reactor analysis
          kernel_montecarlo_reactor_analysis(data, isotopes, indices_data);
        }

        // Update profile data
        device_to_host_time += host_to_device_time;
      }
    }
  }

  #pragma omp target(dataflow)
  __global__ void binary_search_kernel(int size, long iterations, int num_warmups) {
    // Perform binary search
    for (int j=0; j<iterations; ++j) {
      // Perform kernel
      if (in.kernel_id == 0) {
        // Kernel for event-based model
        kernel_event_based(data);
      }
      else if (in.kernel_id == 1) {
        // Kernel for Monte Carlo reactor analysis
        kernel_montecarlo_reactor_analysis(data);
      }

      // Update profile data
      host_to_device_time += device_to_host_time;
    }
  }

  #pragma omp target(map(tofrom:results, indices))
  __global__ void launch_binary_search_kernel(int size) {
    // Perform binary search
    for (int j=0; j<in.lookups; ++j) {
      // Perform kernel
      if (in.kernel_id == 0) {
        // Kernel for event-based model
        kernel_event_based(data);
      }
      else if (in.kernel_id == 1) {
        // Kernel for Monte Carlo reactor analysis
        kernel_montecarlo_reactor_analysis(data);
      }

      // Update profile data
      device_to_host_time += host_to_device_time;
    }
  }

  #pragma omp target(map(from:results, indices))
  __global__ void finalization_kernel(int size) {
    // Finalize variables and copy results back to host
    for (int j=0; j<in.lookups; ++j) {
      long num_isotopes = in.n_isotopes[in.grid_type];
      double* isotopes = results + idx * num_isotopes;

      // Copy results back to host
      for (int k=0; k<num_isotopes; ++k) {
        data[idx*num_isotopes + j * in.grid_type] = isotopes[k];
      }
    }

    free(results);
    free(indices);
  }

  // Launch finalization kernel on GPU
  launch_binary_search_kernel(in.num_particles);

  // Finalize variables and copy results back to host
  for (int i=0; i<in.num_particles; ++i) {
    int material_index = in.particles[i].material;
    double energy = in.particles[i].energy;

    // Check if particle is found
    if (results[material_index*in.lookups+energy] == 1.0) {
      // Particle is found, update material index and energy
      in.particles[i].material = material_index;
      in.particles[i].energy = energy;
    }
  }

  // Finalize variables and copy results back to host
  finalization_kernel(in.num_particles);
}