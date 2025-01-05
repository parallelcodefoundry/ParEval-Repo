#ifndef XSBENCH_SHARED_HEADER_HPP
#define XSBENCH_SHARED_HEADER_HPP

#include <Kokkos_Core.hpp>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <string>
#include <fstream> //for file output

// Header for shared utilities across XSBench versions

struct Inputs {
    int nthreads;
    long n_isotopes;
    long n_gridpoints;
    int lookups;
    std::string HM;
    int grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
    int hash_bins;
    int particles;
    int simulation_method;
    int binary_mode;
    int kernel_id;
    int num_iterations;
    int num_warmups;
    std::string filename;
};

struct Profile {
    double device_to_host_time;
    double kernel_time;
    double host_to_device_time;
};


// Kokkos View for Profile struct.  Allows for parallel access to timing data.
using ProfileView = Kokkos::View<Profile*, Kokkos::HostSpace>;

// Kokkos View for Inputs struct
using InputsView = Kokkos::View<Inputs*, Kokkos::HostSpace>;

inline void print_profile(const Profile& profile, const Inputs& in) {
  std::ofstream outputFile;
  if (!in.filename.empty()) {
    outputFile.open(in.filename);
    outputFile << "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n";
    outputFile << profile.host_to_device_time*1000 << "," 
               << profile.kernel_time*1000 << "," 
               << profile.device_to_host_time*1000 << "," 
               << in.num_iterations << "," 
               << in.num_warmups << "\n";
    outputFile.close();
  } else {
    std::cout << "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n";
    std::cout << profile.host_to_device_time*1000 << "," 
              << profile.kernel_time*1000 << "," 
              << profile.device_to_host_time*1000 << "," 
              << in.num_iterations << "," 
              << in.num_warmups << "\n";
  }
}


#endif // XSBENCH_SHARED_HEADER_H