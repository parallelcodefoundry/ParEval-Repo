// microXORh header file

#ifndef MICROXORH_HPP
#define MICROXORH_HPP

#include <iostream>
#include <random>
#include <vector>
#include <omp.h>

/* Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
   Note that we only consider neighbors and not input_{i,j} when computing output_{i,j}.
   input and output are NxN grids of ints in row-major.
   Use OpenMP GPU offload to compute in parallel.
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
void cellsXOR(int* input, int* output, size_t N, size_t threadsPerTeam, size_t numTeams) {
#pragma omp target teams distribute parallel for collapse(2) map(to: input[0:N*N]) map(from: output[0:N*N]) num_teams(numTeams) thread_limit(threadsPerTeam)
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      int count = 0;
      if (i > 0 && input[(i-1)*N + j] == 1) count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) count++;
      if (j > 0 && input[i*N + (j-1)] == 1) count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) count++;
      output[i*N + j] = count == 1;
    }
  }
}

#endif
