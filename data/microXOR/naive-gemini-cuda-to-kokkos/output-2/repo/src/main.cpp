// microXOR driver

#include "microXOR.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>
#include <random>

using int_type = int;


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

  Kokkos::ScopeGuard guard;


  Kokkos::View<int_type*, Kokkos::HostSpace> input("input", N * N);
  Kokkos::View<int_type*, Kokkos::HostSpace> output("output", N * N);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int_type> dis(0, 1);
  for (size_t i = 0; i < N * N; i++) {
    input(i) = dis(gen);
  }

  Kokkos::View<int_type*, Kokkos::DefaultExecutionSpace> d_input("d_input", N * N);
  Kokkos::View<int_type*, Kokkos::DefaultExecutionSpace> d_output("d_output", N * N);

  Kokkos::deep_copy(d_input, input);

  dim3 threadsPerBlock(blockEdge, blockEdge);
  dim3 numBlocks((N + threadsPerBlock.x - 1) / threadsPerBlock.x,
                 (N + threadsPerBlock.y - 1) / threadsPerBlock.y);

  Kokkos::parallel_for(Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0,0},{N,N}, {blockEdge, blockEdge}),
                        cellsXOR(d_input, d_output, N));

  Kokkos::deep_copy(output, d_output);

  // Validate the output
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      int count = 0;
      if (i > 0 && input((i-1)*N + j) == 1) count++;
      if (i < N-1 && input((i+1)*N + j) == 1) count++;
      if (j > 0 && input(i*N + (j-1)) == 1) count++;
      if (j < N-1 && input(i*N + (j+1)) == 1) count++;
      if (count == 1) {
        if (output(i*N + j) != 1) {
          std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
          return 1;
        }
      } else {
        if (output(i*N + j) != 0) {
          std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
          return 1;
        }
      }
    }
  }
  std::cout << "Validation passed." << std::endl;
  return 0;
}