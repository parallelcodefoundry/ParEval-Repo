// microXOR driver

#include "microXOR.cuh"

void cleanup(int *input, int *output, int *d_input, int *d_output) {
  delete[] input;
  delete[] output;
  free(d_input);
  free(d_output);
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

  int *d_input, *d_output;
  d_input = (int*) malloc(N * N * sizeof(int));
  d_output = (int*) malloc(N * N * sizeof(int));

  #pragma omp target data map(to: input[0:N*N]) map(alloc: d_input[0:N*N])
  {
    #pragma omp target teams distribute parallel for 
    for (size_t i = 0; i < N * N; i++) {
      d_input[i] = input[i];
    }

    #pragma omp target teams distribute parallel for map(to: d_input[0:N*N]) map(from: d_output[0:N*N])
    for (size_t i = 0; i < N; i++) {
      for (size_t j = 0; j < N; j++) {
        int count = 0;
        if (i > 0 && d_input[(i-1)*N + j] == 1) count++;
        if (i < N-1 && d_input[(i+1)*N + j] == 1) count++;
        if (j > 0 && d_input[i*N + (j-1)] == 1) count++;
        if (j < N-1 && d_input[i*N + (j+1)] == 1) count++;
        d_output[i*N + j] = (count == 1) ? 1 : 0;
      }
    }
  }

  #pragma omp target data map(from: d_output[0:N*N])
  {
    #pragma omp target teams distribute parallel for
    for (size_t i = 0; i < N * N; i++) {
      output[i] = d_output[i];
    }
  }

  /*
  for (int i = 0; i < N*N; i++) {
    std::cout << output[i] << " ";
    if (i % N == N - 1) std::cout << std::endl;
  }
  */

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
          cleanup(input, output, d_input, d_output);
          return 1;
        }
      } else {
        if (output[i*N + j] != 0) {
          std::cerr << "Validation failed at (" << i << ", " << j << ")" << std::endl;
          cleanup(input, output, d_input, d_output);
          return 1;
        }
      }
    }
  }
  std::cout << "Validation passed." << std::endl;
  cleanup(input, output, d_input, d_output);
  return 0;
}