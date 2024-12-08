#include "XSbench_shared_header.h"
#include <omp.h>
#include <stdio.h>

int main(int argc, char **argv) {
  // Parse command-line arguments
  Inputs in;
  parse_args(argc, argv, &in);

  if (in.simulation_method == EVENT_BASED_METHOD) {
    #pragma omp target data map(to: in)
    {
      // Perform sorting and initialization here
      sort_particles(&in);
      init_data_structures(&in);
    }
  }

  #pragma omp target teams distribute parallel for
  {
    // Main simulation loop
    if (in.simulation_method == EVENT_BASED_METHOD) {
      perform_event_based_simulation(&in);
    } else {
      // Default to event-free simulation
      perform_event_free_simulation(&in);
    }
  }

  Profile profile;
  #pragma omp target update from(in)
  {
    // Calculate performance metrics
    calculate_performance_metrics(&profile, &in);
  }

  print_profile(profile, in);

  return 0;
}