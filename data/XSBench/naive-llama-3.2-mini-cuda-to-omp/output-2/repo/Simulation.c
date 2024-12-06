#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "XSBEConfig.h"
#include "XSBEcommon_functions.h"

#ifdef _OPENMP
#pragma omp target map(ToDevice, ToDevice) parallel for schedule(static)
#endif

void XSBenchSimulate(const Inputs &in, Profile &profile) {
  #ifdef _OPENMP
    int num_threads = in.nthreads;
    #pragma omp parallel for num_threads(num_threads) schedule(static)
    for (int i = 0; i < in.particles; i++) {
      // particle simulation loop here...
    }
  #else
    // sequential execution here
    for (int i = 0; i < in.particles; i++) {
      // particle simulation loop here...
    }
  #endif

#ifdef XSBENCH_USE_PROFILING
  clock_t start_time, end_time;
  start_time = clock();
  // ... do some work ...
  end_time = clock();
  double time_spent = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  profile.kernel_time += time_spent;

  start_time = clock();
  // ... another piece of work ...
  end_time = clock();
  double time_spent2 = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  profile.device_to_host_time += time_spent2;
#endif

#ifdef XSBENCH_USE_DEVICE Profiling
  #ifdef CUDA_HOST_DEVICE
    __device__ void device_kernel() {
      // kernel code here...
    }
  #else
    int num_gridpoints = in.n_gridpoints;
    for (int i = 0; i < num_gridpoints; i++) {
      // grid point simulation loop here...
    }
  #endif

#ifdef XSBENCH_USE_DEVICE Profiling (cont.)
  start_time = clock();
  device_kernel();
  end_time = clock();
  double time_spent_device = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  profile.kernel_time += time_spent_device;

  start_time = clock();
  __host__ void ToDevice() {
    // host-to-device transfer here...
  }
  end_time = clock();
  double time_spent_device_to_host = (double)(end_time - start_time) / CLOCKS_PER_SEC;
  profile.device_to_host_time += time_spent_device_to_host;

#endif
}

void XSBenchInit(const Inputs &in, Profile &profile) {
#ifdef _OPENMP
  #pragma omp target map(ToDevice, ToDevice)
#endif

  // initialize some global variables here...

  profile.num_iterations = in.num_iterations;
  profile.num_warmups = in.num_warmups;

}

void XSBenchGetProfile(const Inputs &in, Profile &profile) {
#ifdef _OPENMP
  #pragma omp target map(ToDevice, ToDevice)
#endif

  // get some values from memory here...
  profile.host_to_device_time = devToHostTime;
  profile.kernel_time = kernelTime;
  profile.device_to_host_time = deviceToHostTime;

}

void XSBenchPrintProfile(const Profile &profile) {
#ifdef _OPENMP
  #pragma omp target map(ToDevice, ToDevice)
#endif

  print_profile(profile, Inputs());
}