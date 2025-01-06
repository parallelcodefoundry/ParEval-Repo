// microXOR driver

#include <Kokkos_Core.hpp>
#include "microXOR.hpp"

void cleanup(int *input, int *output) {
  delete[] input;
  delete[] output;
}

int main(int argc, char **argv) {
  Kokkos::initialize(argc, argv);
  {
    if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " N blockEdge" << std::endl;
      Kokkos::finalize();
      return 1;
    }

    size_t N = std::stoi(argv[1]);
    size_t blockEdge = std::stoi(argv[2]);

    if (N % blockEdge != 0) {
      std::cerr << "N must be divisible by blockEdge" << std::endl;
      Kokkos::finalize();
      return 1;
    }
    if (blockEdge < 2 || blockEdge > 32) {
      std::cerr << "blockEdge must be between 2 and 32" << std::endl;
      Kokkos::finalize();
      return 1;
    }
    if (N < 4) {
      std::cerr << "N must be at least 4" << std::endl;
      Kokkos::finalize();
      return 1;
    }

    // Allocate input and output arrays
    int *input = new int[N * N];
    int *output = new int[N * N];
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(0, 1);
    for (size_t i = 0; i < N * N; i++) {
      input[i] = dis(gen);
    }

    // Create Kokkos views for input and output
    Kokkos::View<int**> d_input("d_input", N, N);
    Kokkos::View<int**> d_output("d_output", N, N);

    // Copy data from host to device
    Kokkos::parallel_for("CopyInput", N, KOKKOS_LAMBDA(const size_t i) {
      for (size_t j = 0; j < N; j++) {
        d_input(i, j) = input[i * N + j];
      }
    });

    // Launch the Kokkos kernel
    Kokkos::parallel_for("cellsXOR", Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0, 0}, {N, N}), KOKKOS_LAMBDA(const size_t i, const size_t j) {
      int count = 0;
      if (i > 0 && d_input(i - 1, j) == 1) count++;
      if (i < N - 1 && d_input(i + 1, j) == 1) count++;
      if (j > 0 && d_input(i, j - 1) == 1) count++;
      if (j < N - 1 && d_input(i, j + 1) == 1) count++;
      d_output(i, j) = (count == 1) ? 1 : 0;
    });

    // Copy data from device to host
    Kokkos::parallel_for("CopyOutput", N, KOKKOS_LAMBDA(const size_t i) {
      for (size_t j = 0; j < N; j++) {
        output[i * N + j] = d_output(i, j);
      }
    });

    // Validate the output
    for (size_t i = 0; i < N; i++) {
      for (size_t j = 0; j < N; j++) {
        int count = 0;
        if (i > 0 && input[(i - 1) * N + j] == 1) count++;
        if (i < N - 1 && input[(i + 1) * N + j] == 1) count++;
        if (j > 0 && input[i * N + (j - 1)] == 1) count++;
        if (j < N - 1 && input[i * N + (j + 1)] == 1) count++;
        if (count == 1) {
          if (output[i * N + j] != 1) {
            std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
            cleanup(input, output);
            Kokkos::finalize();
            return 1;
          }
        } else {
          if (output[i * N + j] != 0) {
            std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
            cleanup(input, output);
            Kokkos::finalize();
            return 1;
          }
        }
      }
    }
    std::cout << "Validation passed." << std::endl;
    cleanup(input, output);
  }
  Kokkos::finalize();
  return 0;
}