// microXOR header file

#ifndef MICROXOR_HPP
#define MICROXOR_HPP

#include <iostream>
#include <random>
#include <Kokkos_Core.hpp>

// Kokkos kernel for cellsXOR
template <typename ExecutionSpace>
struct cellsXORFunctor {
  const int* input;
  int* output;
  size_t N;

  cellsXORFunctor(const int* input_, int* output_, size_t N_) : input(input_), output(output_), N(N_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(int i, int j) const {
    if (i < N && j < N) {
      int count = 0;
      if (i > 0 && input[(i - 1) * N + j] == 1) count++;
      if (i < N - 1 && input[(i + 1) * N + j] == 1) count++;
      if (j > 0 && input[i * N + (j - 1)] == 1) count++;
      if (j < N - 1 && input[i * N + (j + 1)] == 1) count++;
      output[i * N + j] = (count == 1) ? 1 : 0;
    }
  }
};


template <typename ExecutionSpace>
void cellsXOR(const int* input, int* output, size_t N);


#endif