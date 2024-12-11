// microXOR driver (translated to OpenMP-offload execution model)

#include "microXOR.cuh"

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Usage: ./microXOR.exe <N> <block_size>" << std::endl;
    return 1;
  }

  int N = atoi(argv[1]);
  int block_size = atoi(argv[2]);

  int* input = new int[N * N];
  int* output = new int[N * N];

  for (int i = 0; i < N*N; i++) {
    input[i] = rand() % 2;
  }

  // Create an OpenMP team and offload to a device
  #pragma omp target map( input[:N*N], output[:N*N]) teams num_teams(1)
  {
    for (int i = 0; i < N * block_size; i += block_size) {
      for (int j = 0; j < N * block_size; j += block_size) {
        // Create a sub-team for this tile
        #pragma omp team distribute parallel if(N>64)
        {
          int count;
          int index;

          // Each thread computes the output value for one cell
          #pragma omp parallel for private(count, index)
          for (int k = i; k < min(i + block_size, N); ++k) {
            for (int l = j; l < min(j + block_size, N); ++l) {
              count = 0;
              if (k > 0 && input[(k-1)*N + l] == 1) count++;
              if (k < N-1 && input[(k+1)*N + l] == 1) count++;
              if (l > 0 && input[k*N + (l-1)] == 1) count++;
              if (l < N-1 && input[k*N + (l+1)] == 1) count++;
              output[k*N + l] = (count == 1) ? 1 : 0;
            }
          }
        }
      }
    }
  }

  // Validate the output
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      int count = 0;
      if (i > 0 && input[(i-1)*N + j] == 1) count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) count++;
      if (j > 0 && input[i*N + (j-1)] == 1) count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) count++;
      if (count == 1) {
        if (output[i*N + j] != 1) {
          std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
          delete[] input;
          delete[] output;
          return 1;
        }
      } else {
        if (output[i*N + j] != 0) {
          std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
          delete[] input;
          delete[] output;
          return 1;
        }
      }
    }
  }
  std::cout << "Validation passed." << std::endl;

  delete[] input;
  delete[] output;
  return 0;
}