#include <Kokkos_Core.hpp>
#include "microXOR.hpp"

/* Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
   Note that we only consider neighbors and not input_{i,j} when computing output_{i,j}.
   input and output are NxN grids of ints in row-major.
   Use Kokkos to compute in parallel. The kernel is executed on an NxN grid of threads.
*/
struct CellsXOR {
  const Kokkos::View<const int**> input;
  Kokkos::View<int**> output;
  size_t N;

  CellsXOR(const Kokkos::View<const int**> input, Kokkos::View<int**> output, size_t N)
      : input(input), output(output), N(N) {}

  KOKKOS_INLINE_FUNCTION
  void operator()(const int i, const int j) const {
    if (i < N && j < N) {
      int count = 0;
      if (i > 0 && input(i - 1, j) == 1) count++;
      if (i < N - 1 && input(i + 1, j) == 1) count++;
      if (j > 0 && input(i, j - 1) == 1) count++;
      if (j < N - 1 && input(i, j + 1) == 1) count++;
      output(i, j) = (count == 1) ? 1 : 0;
    }
  }
};

void runCellsXOR(Kokkos::View<const int**> input, Kokkos::View<int**> output, size_t N) {
  Kokkos::parallel_for("CellsXOR", Kokkos::RangePolicy<>(0, N * N), KOKKOS_LAMBDA(const int idx) {
    int i = idx / N;
    int j = idx % N;
    CellsXOR cellsXOR(input, output, N);
    cellsXOR(i, j);
  });
}