NVCC ?= nvcc
CUDA_ARCH ?= sm_80
SRC = src
INC = include
OBJ = obj
NVCCFLAGS ?= -O3 -arch=$(CUDA_ARCH) -I$(INC) -std=c++17
LDFLAGS ?= -lcudart

CUFILES = $(SRC)/microXOR.cu $(SRC)/main.cu
DEPS = $(INC)/microXOR.cuh
OBJS = $(CUFILES:$(SRC)/%.cu=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(NVCC) $(NVCCFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cu $(DEPS) | obj
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean
```

**OpenMP Offload Translation:**

To translate the code to OpenMP Offload, replace the CUDA-specific code with OpenMP directives. Here's a breakdown:

1. **Header File (microXOR.cuh):** No changes required.

2. **Kernel File (microXOR.cu):**

   - Replace `__global__ void` with `void` and add `#pragma omp target teams distribute parallel for` directive before the loop.
   - Remove `blockIdx`, `blockDim`, `threadIdx`.
   - Access the global memory using `input` and `output` directly.

```c++
#include <cuda_runtime.h>
#include "microXOR.cuh"

/* Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
   Note that we only consider neighbors and not input_{i,j} when computing output_{i,j}.
   input and output are NxN grids of ints in row-major.
   Use OpenMP Offload to compute in parallel. 
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
  #pragma omp target teams distribute parallel for
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      int count = 0;
      if (i > 0 && input[(i-1)*N + j] == 1) count++;
      if (i < N-1 && input[(i+1)*N + j] == 1) count++;
      if (j > 0 && input[i*N + (j-1)] == 1) count++;
      if (j < N-1 && input[i*N + (j+1)] == 1) count++;
      output[i*N + j] = (count == 1) ? 1 : 0;
    }
  }
}
```

3. **Main File (main.cu):**

   - Replace `cudaMalloc` with `malloc` and `cudaFree` with `free`.
   - Replace `cudaMemcpy` with `memcpy`.
   - Remove `dim3` declarations.
   - Call `cellsXOR` directly without launch parameters.
   - Use `#pragma omp target data map(input, output) ...` to map the data to the device before the kernel call.

```c++
#include "microXOR.cuh"

#include <iostream>
#include <random>

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

  int *d_input = (int*) malloc(N * N * sizeof(int));
  int *d_output = (int*) malloc(N * N * sizeof(int));

  memcpy(d_input, input, N * N * sizeof(int));

  #pragma omp target data map(input, output)
  {
    cellsXOR(d_input, d_output, N);
  }

  memcpy(output, d_output, N * N * sizeof(int));

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
```

4. **Makefile:**

   - Remove `NVCC` and `CUDA_ARCH` variables.
   - Change `NVCCFLAGS` to `CXXFLAGS` and set `-fopenmp` flag.
   - Change `LDFLAGS` to `LDLIBS` and set `-lgomp` flag.
   - Change `$(OBJ)/%.o: $(SRC)/%.cu $(DEPS) | obj` to `$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj`.

```makefile
CXX ?= g++
SRC = src
INC = include
OBJ = obj
CXXFLAGS ?= -O3 -I$(INC) -std=c++17 -fopenmp
LDLIBS ?= -lgomp

CUFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.cuh
OBJS = $(CUFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDLIBS)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean