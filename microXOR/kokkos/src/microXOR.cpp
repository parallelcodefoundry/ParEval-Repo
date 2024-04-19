#include "microXOR.hpp"

/* Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
   Note that we only consider neighbors and not input_{i,j} when computing output_{i,j}.
   input and output are NxN grids of ints in row-major.
   Use Kokkos to compute in parallel. Assume Kokkos has already been initialized.
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
void cellsXOR(Kokkos::View<const int**> &input, Kokkos::View<int**> &output, size_t N, size_t threadsPerTeam, size_t numTeams) {
  Kokkos::parallel_for("cellsXOR", Kokkos::MDRangePolicy<Kokkos::Rank<2>>({0, 0}, {N, N}),
    KOKKOS_LAMBDA (const int i, const int j) {
      for (size_t i = 0; i < N; i++) {
	for (size_t j = 0; j < N; j++) {
	  int count = 0;
	  if (i > 0 && input(i-1, j) == 1) count++;
	  if (i < N-1 && input(i+1, j) == 1) count++;
	  if (j > 0 && input(i, j-1) == 1) count++;
	  if (j < N-1 && input(i, j+1) == 1) count++;
	  output(i, j) = count == 1;
	}
      }
  });
}
