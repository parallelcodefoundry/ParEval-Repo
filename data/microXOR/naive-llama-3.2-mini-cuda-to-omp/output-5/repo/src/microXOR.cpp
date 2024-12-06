// microXOR kernel (OpenMP-Offload)

#include <iostream>
#include <random>
#include "microXOR.h"

/* Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
   Note that we only consider neighbors and not input_{i,j} when computing output_{i,j}.
   Use OpenMP to compute in parallel. The function is called from host code.
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
void cellsXOR(const int *input, int *output, size_t N) {
    #pragma omp parallel for
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            int count = 0;
            if (i > 0 && input[(i-1)*N + j] == 1) count++;
            if (i < N-1 && input[(i+1)*N + j] == 1) count++;
            if (j > 0 && input[i*N + (j-1)] == 1) count++;
            if (j < N-1 && input[i*N + (j+1)] == 1) count++;
            output[i*N + j] = (count == 1) ? 1 : 0;
        }
    }
}
