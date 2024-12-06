#include "XSBench_shared_header.h"

// OpenMP offload main function
__global__ void kernel_main(void* main_input, Inputs *main_inputs) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  // Calculate number of warmup iterations based on number of threads and warm-up percentage.
  int warmup_iterations = (int)(ceil((double)main_inputs->num_warmups / (double)blockDim.x)) * main_inputs->warmup_percentage;

  // First offload: Warm up simulation
  for (int i = 0; i < warmup_iterations; ++i) {
    // Offload the kernel to the device, which is not suitable for all kernels.
    __shared__ double device_to_host_time;
    #pragma omp target(map(to:device_to_host_time)) map(ensurelastdimension:device_to_host_time)
    __device__ void offload_device_to_host_time(double *t) {
      // The offloaded function executes on the device and can only read from the host.
      t[0] = 0;
    }
    #pragma omp target(map(to:main_inputs)) map(ensurelastdimension:main_inputs)
    __device__ void offload_simulate(double *simulation_data, Inputs *in) {
      // Simulation function executes on the device and can read from both host and device.
      // This function should be modified to perform Monte Carlo reactor analysis simulation as in XSBench's original implementation.
    }
  }

  // Offload warm-up iterations
  offload_device_to_host_time(&device_to_host_time);
  for (int i = 0; i < main_inputs->num_warmups - warmup_iterations; ++i) {
    // Warm-up simulation continues as normal.
    __shared__ double device_to_host_time;
    #pragma omp target(map(to:device_to_host_time)) map(ensurelastdimension:device_to_host_time)
    __device__ void offload_device_to_host_time(double *t) {
      t[0] = 0;
    }
    #pragma omp target(map(to:main_inputs)) map(ensurelastdimension:main_inputs)
    __device__ void offload_simulate(double *simulation_data, Inputs *in) {
      // Simulation function executes on the device and can read from both host and device.
      // This function should be modified to perform Monte Carlo reactor analysis simulation as in XSBench's original implementation.
    }
  }

  for (int i = 0; i < main_inputs->num_iterations - warmup_iterations; ++i) {
    __shared__ double device_to_host_time;
    #pragma omp target(map(to:device_to_host_time)) map(ensurelastdimension:device_to_host_time)
    __device__ void offload_device_to_host_time(double *t) {
      t[0] = 0;
    }
    #pragma omp target(map(to:main_inputs)) map(ensurelastdimension:main_inputs)
    __device__ void offload_simulate(double *simulation_data, Inputs *in) {
      // Simulation function executes on the device and can read from both host and device.
      // This function should be modified to perform Monte Carlo reactor analysis simulation as in XSBench's original implementation.
    }
  }

  for (int i = 0; i < main_inputs->num_warmups - warmup_iterations; ++i) {
    offload_device_to_host_time(&device_to_host_time);
  }

  #pragma omp target(map(to:main_inputs)) map(ensurelastdimension:main_inputs)
  __global__ void finish_simulate(double *simulation_data, Inputs *in) {
    // Simulation function executes on the device and can read from both host and device.
    // This function should be modified to perform Monte Carlo reactor analysis simulation as in XSBench's original implementation.
    if (idx < main_inputs->num_particles) {
      // Main simulation continues as normal.
    }
  }

  __syncs();
}

int main() {
  int num_threads;
#ifdef CPU
  num_threads = omp_get_max_threads();
#else
  num_threads = 1;  // No threads on GPU
#endif

  cudaDeviceProp deviceProp;
  cudaGetDevice(0, &deviceProp);

  if (deviceProp.canDoublePrecision) {
    double *simulation_data;
    Inputs inputs;

    cudaMalloc((void **)&simulation_data, main_inputs->n_gridpoints * sizeof(double));

    for (int i = 0; i < num_threads; ++i) {
      int idx = i % main_inputs->num_particles;
      kernel_main<<<main_inputs->nthreads, main_inputs->nthreads>>>(simulation_data, &inputs);
    }

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
      fprintf(stderr, "Error: %s", cudaGetErrorString(err));
      return 1;
    }

    for (int i = 0; i < main_inputs->num_particles; ++i) {
      if (idx == i)
        finish_simulate(simulation_data, &inputs);
    }

    cudaFree(simulation_data);

#ifdef GPU
  printf("Simulation complete. Time elapsed: %f seconds\n", main_inputs->time_elapsed);
#endif

#else
  // If not running on GPU, run the simulation in host.
  double *simulation_data;
  Inputs inputs;

  cudaMalloc((void **)&simulation_data, main_inputs->n_gridpoints * sizeof(double));

#ifdef CPU
  for (int i = 0; i < num_threads; ++i) {
    kernel_main<<<main_inputs->nthreads, main_inputs->nthreads>>>(simulation_data, &inputs);
  }
#else
  // If running on CPU, the simulation can't be offloaded to a device.
  #ifdef DEBUG
  printf("Simulation not supported on GPU\n");
  return 1;
  #else
  // If debugging mode is enabled and simulation not supported on GPU, exit program.
  return 0;
  #endif
#endif

  for (int i = 0; i < main_inputs->num_particles; ++i) {
    if (idx == i)
      finish_simulate(simulation_data, &inputs);
  }

  cudaFree(simulation_data);

#ifdef CPU
  printf("Simulation complete. Time elapsed: %f seconds\n", clock());
#else
  // If running on GPU, return immediately.
  return 0;
#endif

  return 0;
}

// Main function
int main() {
  Inputs main_inputs;
  int num_particles = 1000000;  // Example number of particles in the simulation
  double simulation_data[num_particles];
  for (int i = 0; i < num_particles; ++i) {
    simulation_data[i] = rand() / (double)RAND_MAX;
  }
  main_inputs.n_gridpoints = num_particles;

#ifdef CPU
  main_inputs.time_elapsed = clock();
#else
  // If running on GPU, initialize the time elapsed before running the simulation.
  main_inputs.time_elapsed = 0.0;
#endif

  cudaMalloc((void **)&simulation_data, main_inputs.n_gridpoints * sizeof(double));
  for (int i = 0; i < num_particles; ++i) {
    if (i % main_inputs.n_particles == 0)
      simulation_data[i] = rand() / (double)RAND_MAX;
  }
  main_inputs.time_elapsed += clock();

#ifdef CPU
  #pragma omp parallel for
  for (int i = 0; i < num_particles; ++i) {
    if (main_inputs.warmup_percentage > 0)
      // Perform warm-up simulation.
      finish_simulate(simulation_data, &main_inputs);
    else
      main_inputs.simulation_data[i] = simulation_data[i];
  }
#else
  // If running on GPU, run the simulation in parallel using OpenMP.
  for (int i = 0; i < num_particles; ++i) {
    #ifdef DEBUG
    printf("Simulation iteration %d\n", i);
    #endif
    if (main_inputs.warmup_percentage > 0)
      // Perform warm-up simulation.
      finish_simulate(simulation_data, &main_inputs);
    else
      main_inputs.simulation_data[i] = simulation_data[i];
  }
#endif

#ifdef CPU
  for (int i = 0; i < num_particles; ++i) {
    if (main_inputs.warmup_percentage > 0)
      // Perform warm-up simulation.
      finish_simulate(simulation_data, &main_inputs);
    else
      main_inputs.simulation_data[i] = simulation_data[i];
  }
#else
  // If running on GPU, run the simulation in parallel using OpenMP.
  for (int i = 0; i < num_particles; ++i) {
    #ifdef DEBUG
    printf("Simulation iteration %d\n", i);
    #endif
    if (main_inputs.warmup_percentage > 0)
      // Perform warm-up simulation.
      finish_simulate(simulation_data, &main_inputs);
    else
      main_inputs.simulation_data[i] = simulation_data[i];
  }
#endif

#ifdef CPU
  for (int i = 0; i < num_particles; ++i) {
    if (main_inputs.warmup_percentage > 0)
      // Perform warm-up simulation.
      finish_simulate(simulation_data, &main_inputs);
    else
      main_inputs.simulation_data[i] = simulation_data[i];
  }
#else
  // If running on GPU, run the simulation in parallel using OpenMP.
  for (int i = 0; i < num_particles; ++i) {
    #ifdef DEBUG
    printf("Simulation iteration %d\n", i);
    #endif
    if (main_inputs.warmup_percentage > 0)
      // Perform warm-up simulation.
      finish_simulate(simulation_data, &main_inputs);
    else
      main_inputs.simulation_data[i] = simulation_data[i];
  }
#endif

#ifdef CPU
  for (int i = 0; i < num_particles; ++i) {
    if (main_inputs.warmup_percentage > 0)
      // Perform warm-up simulation.
      finish_simulate(simulation_data, &main_inputs);
    else
      main_inputs.simulation_data[i] = simulation_data[i];
  }
#else
  // If running on GPU, run the simulation in parallel using OpenMP.
  for (int i = 0; i < num_particles; ++i) {
    #ifdef DEBUG
    printf("Simulation iteration %d\n", i);
    #endif
    if (main_inputs.warmup_percentage > 0)
      // Perform warm-up simulation.
      finish_simulate(simulation_data, &main_inputs);
    else
      main_inputs.simulation_data[i] = simulation_data[i];
  }
#endif

#ifdef CPU
  printf("Simulation complete. Time elapsed: %f seconds\n", clock() - main_inputs.time_elapsed);
#else
  // If running on GPU, exit program.
  return 0;
#endif

  #ifdef DEBUG
  printf("Simulation not supported on GPU\n");
  return 1;
  #else
  // If debugging mode is enabled and simulation not supported on GPU, exit program.
  return 0;
  #endif

  return 0;
}