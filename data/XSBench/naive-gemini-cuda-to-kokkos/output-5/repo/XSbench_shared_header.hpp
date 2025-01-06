#ifndef XSBENCH_SHARED_HEADER_HPP
#define XSBENCH_SHARED_HEADER_HPP

#include <Kokkos_Core.hpp>
#include <cstdio> // For printf
#include <cstdlib> // For malloc etc
#include <string> // For string manipulation
#include <fstream> // for file operations


// Header for shared utilities across XSBench versions

struct Inputs {
    int nthreads;
    long n_isotopes;
    long n_gridpoints;
    long lookups; //Changed to long to avoid potential overflow
    std::string HM;
    int grid_type; // 0: Unionized Grid (default)    1: Nuclide Grid
    int hash_bins;
    long particles; //Changed to long to avoid potential overflow
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

//Helper function to write profile data to file, handles both file and stdout cases
template <typename Device>
void write_profile_data(const Profile& profile, const Inputs& in, const std::string& filename) {
  std::ofstream output;
  if (filename.empty()) {
    output.rdbuf(std::cout.rdbuf()); // Redirect to cout if filename is empty
  }
  else {
    output.open(filename);
    if (!output.is_open()) {
      std::cerr << "Error opening file: " << filename << std::endl;
      return; //Handle file opening error
    }
  }

  output << "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n";
  output << profile.host_to_device_time * 1000 << ","
         << profile.kernel_time * 1000 << ","
         << profile.device_to_host_time * 1000 << ","
         << in.num_iterations << ","
         << in.num_warmups << "\n";
  output.close();
}



template<class ExecutionSpace>
void print_profile(const Profile& profile, const Inputs& in){
  write_profile_data<ExecutionSpace>(profile, in, in.filename);
}


#endif // XSBENCH_SHARED_HEADER_H