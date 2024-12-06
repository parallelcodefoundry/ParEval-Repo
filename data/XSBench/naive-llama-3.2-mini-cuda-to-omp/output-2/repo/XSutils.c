// Header for shared utilities across XSBench versions

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define max_threads 32

int num_nuclei;
double time_per_lookup;

inline double compute_num_nuclei() {
  return (double)num_nuclei / max_threads * 1.0;
}

inline void offload_lookup(int idx, int start_idx, int end_idx, __device__ int* lookup_results) {
#pragma omp target teams distribute parallel for num_teams(max_threads) num_gangs(1)
  for(int i=start_idx; i<end_idx; i++) {
    if(idx >= start_idx && idx < end_idx) {
      lookup_results[i] = lookupResults[idx];
    }
  }
}

inline void offload_compute_time() {
  __device__ int* device_data;
  __shared__ int shared_data[max_threads];

  // Copy data to device
  cudaMemcpy(device_data, results, sizeof(results), cudaMemcpyHostToDevice);

  // Initialize global variables on host and wait for all threads to finish
  double host_to_device_time = 0.0;

  #pragma omp target map(tofrom:device_data, shared:shared_data)
  {
    #pragma omp parallel for num_threads(max_threads) 
      for(int i=0; i<num_nuclei; i++) {
        if(i % max_threads == 0) {
          __syncthreads();
        }

        // Perform computation on device
        int idx = i;
        offload_lookup(idx, start_idx, end_idx, lookupResults + idx);

        if (idx < start_idx || idx >= end_idx) {
          results[i] = 0.0; // initialize to zero
        }
      }

      __syncthreads();

      double device_to_host_time = 0.0;
      for(int i=0; i<max_threads; i++) {
        double temp = device_data[i];
        cudaDeviceSynchronize();
        host_to_device_time += temp;

        // Reset local data
        shared_data[i] = 0;
        __syncthreads();
      }
    }

    // Copy result back to host
    cudaMemcpy(results, device_data, sizeof(device_data), cudaMemcpyDeviceToHost);

    #pragma omp target map(tofrom:results) 
    {
      for(int i=0; i<num_nuclei; i++) {
        if(i % max_threads == 0) {
          __syncthreads();
        }

        // Perform time-constant calculation on device
        double time_constant = compute_num_nuclei() * lookup_constant;
        results[i] += time_per_lookup * time_constant;

        if (results[i] < 0.0) {
          results[i] = 0.0; // clamp to zero
        }
      }

      __syncthreads();

      // Wait for all threads to finish
      __syncthreads();
    }

    // Calculate total time spent on computation
    double kernel_time = host_to_device_time + device_to_host_time;
  }

  cudaMemcpy(results, results, sizeof(results), cudaMemcpyHostToDevice);

  time_per_lookup = max(0.0001, kernel_time / num_nuclei);
}

inline void time_results(int idx) {
  __device__ int* device_data;

#pragma omp target map(tofrom:device_data)
{
  // Copy data to device
  cudaMemcpy(device_data, results, sizeof(results), cudaMemcpyHostToDevice);

  // Perform computation on device
  offload_compute_time();

  // Reset local data on host
  device_data[0] = 0;
}

// Rest of XSutils.cu remains the same