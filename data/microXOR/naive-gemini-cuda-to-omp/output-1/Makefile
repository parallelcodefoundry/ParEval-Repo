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

**OpenMP Offload Translations:**

**File: include/microXOR.cuh**

```cpp
#ifndef MICROXOR_CUH
#define MICROXOR_CUH

#include <iostream>
#include <random>

#ifdef __cplusplus
extern "C" {
#endif

void cellsXOR(const int *input, int *output, size_t N);

#ifdef __cplusplus
}
#endif

#endif
```

**File: src/microXOR.cu**

```cpp
#include "microXOR.cuh"

#include <omp.h>

/* Set every cell's value to 1 if it has exactly one neighbor that's a 1. Otherwise set it to 0.
   Note that we only consider neighbors and not input_{i,j} when computing output_{i,j}.
   input and output are NxN grids of ints in row-major.
   Use OpenMP Offload to compute in parallel. The kernel is launched on an NxN grid of threads.
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
  #pragma omp target teams distribute parallel for collapse(2)
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

**File: src/main.cu**

```cpp
#include "microXOR.cuh"

#include <iostream>
#include <random>

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

  #pragma omp target data map(to: input[0:N*N]) map(from: output[0:N*N])
  {
    cellsXOR(input, output, N);
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
```

**File: Makefile**

```makefile
CC ?= g++
CXX ?= g++
OPENMP_FLAGS ?= -fopenmp
CFLAGS ?= -O3 -Wall
CXXFLAGS ?= -O3 -Wall -std=c++17

SRC = src
INC = include
OBJ = obj
EXE = microXOR

all: $(EXE)

$(EXE): $(OBJ)/microXOR.o $(OBJ)/main.o
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -o $(EXE) $(OBJ)/microXOR.o $(OBJ)/main.o

$(OBJ)/microXOR.o: $(SRC)/microXOR.cu $(INC)/microXOR.cuh
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -c -o $(OBJ)/microXOR.o $(SRC)/microXOR.cu

$(OBJ)/main.o: $(SRC)/main.cu $(INC)/microXOR.cuh
	$(CXX) $(CXXFLAGS) $(OPENMP_FLAGS) -c -o $(OBJ)/main.o $(SRC)/main.cu

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJ)/*.o $(EXE)