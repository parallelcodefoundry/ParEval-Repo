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

template <typename ExecutionSpace>
struct cellsXORFunctor {
  const int* input;
  int* output;
  const size_t N;

  cellsXORFunctor(const int* input_, int* output_, size_t N_) : input(input_), output(output_), N(N_) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(const size_t i, const size_t j) const {
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

} // namespace KokkosKernels


template <typename ExecutionSpace>
void cellsXOR(const int* input, int* output, size_t N) {
  Kokkos::RangePolicy<ExecutionSpace> policy(0, N * N);

  Kokkos::parallel_for(policy, KokkosKernels::cellsXORFunctor<ExecutionSpace>(input, output, N));
}


// Explicit instantiation for Kokkos::DefaultExecutionSpace.  You may need to adjust this
// depending on the execution space you intend to use (e.g., Kokkos::Cuda).
template void cellsXOR<Kokkos::DefaultExecutionSpace>(const int* input, int* output, size_t N);
