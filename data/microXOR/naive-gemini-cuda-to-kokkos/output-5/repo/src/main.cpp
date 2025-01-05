// microXOR driver

#include "microXOR.hpp"
#include <Kokkos_Core.hpp>
#include <iostream>
#include <random>

// We need to define the execution space.  We'll use Kokkos::DefaultExecutionSpace here. You might need to adjust this depending on your Kokkos installation and target device.
using ExecutionSpace = Kokkos::DefaultExecutionSpace;

void cleanup(int *input, int *output) {
  delete[] input;
  delete[] output;
}

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

  int *input = new int[N * N];
  int *output = new int[N * N];
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(0, 1);
  for (size_t i = 0; i < N * N; i++) {
    input[i] = dis(gen);
  }


  // Kokkos data structures
  Kokkos::View<int*, Kokkos::HostSpace> h_input("h_input", N * N);
  Kokkos::View<int*, Kokkos::HostSpace> h_output("h_output", N * N);
  Kokkos::View<int*, ExecutionSpace> d_input("d_input", N * N);
  Kokkos::View<int*, ExecutionSpace> d_output("d_output", N * N);

  // Copy data to Kokkos views
  for (size_t i = 0; i < N * N; i++) {
    h_input(i) = input[i];
  }
  Kokkos::deep_copy(d_input, h_input);


  dim3 threadsPerBlock(blockEdge, blockEdge);
  dim3 numBlocks((N + threadsPerBlock.x - 1) / threadsPerBlock.x,
                 (N + threadsPerBlock.y - 1) / threadsPerBlock.y);

  //Kokkos::parallel_for(Kokkos::RangePolicy<ExecutionSpace>(0, N*N), cellsXOR(d_input, d_output, N));

    Kokkos::RangePolicy<ExecutionSpace> policy(0, N*N);
    Kokkos::parallel_for(policy, [&](const int i){
        int row = i / N;
        int col = i % N;
        int count = 0;
        if (row > 0 && d_input((row - 1) * N + col) == 1) count++;
        if (row < N - 1 && d_input((row + 1) * N + col) == 1) count++;
        if (col > 0 && d_input(row * N + col - 1) == 1) count++;
        if (col < N - 1 && d_input(row * N + col + 1) == 1) count++;
        d_output(i) = (count == 1) ? 1 : 0;
    });


  Kokkos::deep_copy(h_output, d_output);

  for (size_t i = 0; i < N * N; i++) {
    output[i] = h_output(i);
  }

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
          cleanup(input, output);
          return 1;
        }
      } else {
        if (output[i*N + j] != 0) {
          std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
          cleanup(input, output);
          return 1;
        }
      }
    }
  }
  std::cout << "Validation passed." << std::endl;
  cleanup(input, output);
  return 0;
}