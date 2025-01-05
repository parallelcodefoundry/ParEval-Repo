// microXOR driver

#include "microXOR.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>
#include <random>

using namespace Kokkos;

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " N blockEdge" << std::endl;
    return 1;
  }

  size_t N = std::stoi(argv[1]);
  size_t blockEdge = std::stoi(argv[2]);

  if (N % blockEdge != 0) {
    std::cerr << "N must be divisible by blockEdge" << std::endl;
    return 1;
  }
  if (blockEdge < 2 || blockEdge > 32) {
    std::cerr << "blockEdge must be between 2 and 32" << std::endl;
    return 1;
  }
  if (N < 4) {
    std::cerr << "N must be at least 4" << std::endl;
    return 1;
  }

  // Allocate input data on the host
  int *input = new int[N * N];
  int *output = new int[N * N];
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(0, 1);
  for (size_t i = 0; i < N * N; i++) {
    input[i] = dis(gen);
  }

  // Create Kokkos views
  View<int*, Kokkos::HostSpace> h_input("h_input", N * N);
  View<int*, Kokkos::HostSpace> h_output("h_output", N * N);
  View<int*, Kokkos::DefaultExecutionSpace> d_input("d_input", N * N);
  View<int*, Kokkos::DefaultExecutionSpace> d_output("d_output", N * N);

  // Copy input data to Kokkos views
  Kokkos::deep_copy(h_input, input);
  Kokkos::deep_copy(d_input, h_input);


  // Define execution policy
  int numBlocksX = (N + blockEdge - 1) / blockEdge;
  int numBlocksY = (N + blockEdge - 1) / blockEdge;
  MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {N,N}, {blockEdge, blockEdge});

  // Launch Kokkos kernel
  Kokkos::parallel_for(policy,cellsXOR(d_input, d_output, N));


  // Copy output data back to host
  Kokkos::deep_copy(h_output, d_output);
  for (int i = 0; i < N*N; i++) output[i] = h_output[i];


  // Validate the output
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      int count = 0;
      if (i > 0 && input[(i-1)*N + j] == 1) count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) count++;
      if (j > 0 && input[i*N + (j-1)] == 1) count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) count++;
      if (count == 1) {
        if (output[i*N + j] != 1) {
          std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
          delete[] input;
          delete[] output;
          return 1;
        }
      } else {
        if (output[i*N + j] != 0) {
          std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
          delete[] input;
          delete[] output;
          return 1;
        }
      }
    }
  }
  std::cout << "Validation passed." << std::endl;
  delete[] input;
  delete[] output;
  return 0;
}