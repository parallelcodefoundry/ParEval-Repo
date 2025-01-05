// microXOR kernel

#include <Kokkos_Core.hpp>
#include "microXOR.hpp"

/* Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
   Note that we only consider neighbors and not input_{i,j} when computing output_{i,j}.
   input and output are NxN grids of ints in row-major.
   Use Kokkos to compute in parallel.
   Example:

   input: [[0, 1, 1, 0],
           [1, 0, 0, 0],
           [0, 0, 0, 0],
           [0, 1, 0, 0]
   output: [[0, 0, 1, 1],
            [1, 0, 0, 1],
            [0, 0, 1, 0],
            [1, 0, 1, 0]]
*/

namespace KokkosKernels {

template <typename T>
struct CellsXORFunctor {
  const T* input;
  T* output;
  size_t N;

  CellsXORFunctor(const T* input_, T* output_, size_t N_) : input(input_), output(output_), N(N_) {}

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

} //namespace KokkosKernels


template <typename ExecutionSpace>
void cellsXOR(const int *input, int *output, size_t N) {
    using KokkosKernels::CellsXORFunctor;
    Kokkos::View<int*, ExecutionSpace> d_input("d_input", N*N);
    Kokkos::View<int*, ExecutionSpace> d_output("d_output", N*N);

    Kokkos::deep_copy(d_input, input);

    Kokkos::MDRangePolicy<ExecutionSpace, Kokkos::Rank<2>> policy({0, 0}, {N,N});
    Kokkos::parallel_for(policy, CellsXORFunctor<int>(d_input.data(), d_output.data(), N));

    Kokkos::deep_copy(output, d_output);
}

