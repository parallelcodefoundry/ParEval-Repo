#include "XSUtils.h"
#include <stdlib.h>
#include <stdio.h>

void xsutils_offload(Inputs *in, Profile *profile) {
  #pragma omp target map(in:in, profile:profile)
  __global__ void utils_kernel() {
    #pragma offload distribute parallel for num_threads(in.nthreads)
    long thread_id = blockIdx.x*blockDim.x+threadIdx.x;
    long isotope = blockDim.x*blockIdy+threadIdx.y;

    if (isotope >= in.n_isotopes) return;

    double energy = 0.0;
    for (int i = 0; i < in.lookups; i++) {
      energy += in.HM[i];
    }

    #pragma offload
    __shared__ double shared_data[in.num_iterations];
    for (long t = 0; t < in.num_iterations; t++) {
      if (in.binary_mode) {
        long idx = thread_id + (isotope * in.nthreads);
        double value = in.grid_type == 1 ? in.HM[idx] : in.HM[energy*idx];
        profile.kernel_time += clock_tval();
      } else {
        shared_data[t] = t;
      }
    }

    #pragma offload
    __shared__ double temp[in.num_iterations];
    for (long i = 0; i < in.num_iterations; i++) {
      if (i >= in.num_warmups) temp[i] = shared_data[i];
    }

    #pragma offload distribute parallel for num_threads(in.nthreads)
    for (int j = 0; j < in.particles; j++) {
      long idx = blockIdx.x*blockDim.x+threadIdx.x;
      double value = temp[idx];
      profile.kernel_time += clock_tval();
      if (in.binary_mode) {
        in.HM[value] = idx;
      }
    }

    #pragma offload
    for (long i = 0; i < in.particles; i++) {
      long idx = blockIdx.x*blockDim.x+threadIdx.x;
      double value = temp[idx];
      profile.kernel_time += clock_tval();
      if (in.binary_mode) {
        in.HM[value] = idx;
      }
    }
  }

  #pragma omp target map(in:in, profile:profile)
  utils_kernel();
}

void xsutils_host(Inputs *in, Profile *profile) {
  for (int i = 0; i < in.lookups; i++) {
    double energy = 0.0;
    for (int j = 0; j < in.n_isotopes; j++) {
      if (i == j*in.lookup_factor + 1) energy = in.HM[j];
    }
    profile.device_to_host_time += clock_tval();
  }

  #pragma offload
  double temp[in.num_iterations];
  for (long i = 0; i < in.num_iterations; i++) {
    temp[i] = i;
  }

  #pragma offload distribute parallel for num_threads(in.nthreads)
  for (int j = 0; j < in.particles; j++) {
    double value = temp[j];
    profile.kernel_time += clock_tval();
    if (in.binary_mode) {
      in.HM[value] = j;
    }
  }

  #pragma offload
  for (long i = 0; i < in.num_iterations; i++) {
    long idx = blockIdx.x*blockDim.x+threadIdx.x;
    double value = temp[idx];
    profile.kernel_time += clock_tval();
    if (in.binary_mode) {
      in.HM[value] = idx;
    }
  }
}