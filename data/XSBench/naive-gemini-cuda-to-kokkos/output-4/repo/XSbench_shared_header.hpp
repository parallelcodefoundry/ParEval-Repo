#ifndef XSBENCH_SHARED_HEADER_HPP
#define XSBENCH_SHARED_HEADER_HPP

#include <Kokkos_Core.hpp>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <string>
#include <fstream>


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

//using device_type = Kokkos::DefaultExecutionSpace;
//using host_type = Kokkos::HostSpace;
template <typename ExecutionSpace>
void print_profile(const Profile& profile, const Inputs& in) {
    std::ofstream output;
    if (!in.filename.empty()) {
        output.open(in.filename);
        if (!output.is_open()) {
            std::cerr << "Error opening file: " << in.filename << std::endl;
            return;
        }
        output << "host_to_device_ms,kernel_ms,device_to_host_ms,num_iterations,num_warmups\n";
    }

    auto print_to = [&](auto& out){
        out << profile.host_to_device_time * 1000 << ","
            << profile.kernel_time * 1000 << ","
            << profile.device_to_host_time * 1000 << ","
            << in.num_iterations << ","
            << in.num_warmups << "\n";
    };

    if (!in.filename.empty()) {
        print_to(output);
        output.close();
    } else {
        print_to(std::cout);
    }
}

#endif // XSBENCH_SHARED_HEADER_H