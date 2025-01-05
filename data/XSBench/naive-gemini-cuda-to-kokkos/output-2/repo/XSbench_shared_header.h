#ifndef XSBENCH_SHARED_HEADER_HPP
#define XSBENCH_SHARED_HEADER_HPP

#include <Kokkos_Core.hpp>
#include <cstdio>
#include <cstring> // for strlen, strcpy

// Header for shared utilities across XSBench versions

struct Inputs {
    int nthreads;
    long n_isotopes;
    long n_gridpoints;
    int lookups;
    char *HM;
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

// Kokkos::View declarations for data structures that will be used with Kokkos
//  (Note:  These are only declarations, the actual views are allocated in the 
//          Kokkos version of GridInit.cpp)

// Example:  This shows how a Kokkos::View would replace a regular C++ array.
//           Note the use of Kokkos::LayoutRight for better cache performance on many systems

template <typename T>
using DeviceArray1D = Kokkos::View<T*, Kokkos::LayoutRight, Kokkos::Device>;


template <typename T>
using DeviceArray2D = Kokkos::View<T**, Kokkos::LayoutRight, Kokkos::Device>;


// Function declaration for Kokkos::parallel_for to handle the printing.
//  This function will be implemented in the Kokkos version of io.cpp

void print_profile(const Profile& profile, const Inputs& in);

#endif // XSBENCH_SHARED_HEADER_HPP