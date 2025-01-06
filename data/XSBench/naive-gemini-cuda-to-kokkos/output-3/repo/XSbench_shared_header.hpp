#ifndef XSBENCH_SHARED_HEADER_HPP
#define XSBENCH_SHARED_HEADER_HPP

#include <Kokkos_Core.hpp>
#include <cstdio> //for printf
#include <cstring> //for strlen, strcpy
#include <fstream> //for ofstream

// Header for shared utilities across XSBench versions

struct Inputs {
    int nthreads;
    long n_isotopes;
    long n_gridpoints;
    int lookups;
    char * HM;
    int grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
    int hash_bins;
    int particles;
    int simulation_method;
    int binary_mode;
    int kernel_id;
    int num_iterations;
    int num_warmups;
    char *filename;
};


struct Profile {
  double device_to_host_time;
  double kernel_time;
  double host_to_device_time;
};

//Helper function to print profile data, supports both stdout and file output.
inline void print_profile(const Profile& profile, const Inputs& in) {
  std::ostream& output = (in.filename) ? std::ofstream(in.filename) : std::cout;
  output << "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n";
  output << profile.host_to_device_time * 1000 << ","
         << profile.kernel_time * 1000 << ","
         << profile.device_to_host_time * 1000 << ","
         << in.num_iterations << ","
         << in.num_warmups << "\n";

  if (in.filename) {
    ((std::ofstream&)output).close();
  }
}


#endif // XSBENCH_SHARED_HEADER_H