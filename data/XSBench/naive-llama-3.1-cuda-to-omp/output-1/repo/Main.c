#include "XSbench_shared_header.h"
#include <omp.h>

int main(int argc, char **argv) {
  Inputs inputs;
  Profile profile;

  // Read command line arguments
  for (int i = 1; i <= argc-1; i++) {
    if (strcmp(argv[i], "-m") == 0) inputs.simulation_method = atoi(argv[++i]);
    else if (strcmp(argv[i], "-s") == 0) inputs.lookups = atoi(argv[++i]);
    else if (strcmp(argv[i], "-f") == 0) inputs.filename = argv[++i];
  }

  // Initialize OpenMP
  omp_set_num_threads(1);
  #pragma omp parallel
  {
    // Set local variables
    int thread_id = omp_get_thread_num();
    double device_to_host_time, kernel_time, host_to_device_time;

    // Perform simulation
    profile.device_to_host_time +=omp_get_wtime();
    if (inputs.simulation_method == 0) { // Event-based simulation
      // Initialize data structures
      ...
      #pragma omp parallel for
      {
        // Iterate over particles and perform simulation
        ...
      }
    } else if (inputs.simulation_method == 1) { // Monte Carlo simulation
      // Initialize data structures
      ...
      #pragma omp parallel for
      {
        // Iterate over particles and perform simulation
        ...
      }
    }

    // Copy results back to host
    profile.host_to_device_time +=omp_get_wtime();

    // Store time measurements in shared variables
    device_to_host_time = profile.device_to_host_time;
    kernel_time = profile.kernel_time;
    host_to_device_time = profile.host_to_device_time;

    #pragma omp critical
    {
      // Update global time measurements
      profile.device_to_host_time = device_to_host_time;
      profile.kernel_time = kernel_time;
      profile.host_to_device_time = host_to_device_time;
    }
  }

  // Print output and perform cleanup
  print_profile(profile, inputs);
  free(inputs.HM);

  return 0;
}