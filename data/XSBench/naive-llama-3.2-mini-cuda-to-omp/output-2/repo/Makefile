# XSBench Makefile

# Compiler flags for CUDA device compilation (openMP-offload)
CCFLAGS += -fopenmp -fopencl-icc -O3 -g -Wall -Wextra -pedantic -std=c++11

# Linker flags for CUDA device compilation (openMP-offload)
LDFLAGS += -fopenmp -fopencl-icc -L/usr/local/cuda/lib64/

# Target for compilation
TARGET = xsbench

# Source files
SRC = $(wildcard *.c)

# Object file
OBJ = $(SRC:.c=.o)

# Header files
HDR = xsbench_shared_header.h

# Dependencies
DEPS = $(OBJS) $(HDR)

# C++ compiler
CXX = g++

# CUDA compiler
NVCC = nvcc

# OpenMP-offload compilation flags for C++ source file
OPENMP_CXXFLAGS += -fopenmp -fopencl-icc -O3 -g -Wall -Wextra -pedantic -std=c++11

# Linker flags for CUDA device compilation (openMP-offload)
LDFLAGS_OPENMP = $(LDFLAGS) -fopenmp -fopencl-icc -O3 -g -Wall -Wextra -pedantic -std=c++11

# OpenMP-offload compiler
OPENMP_CXX = g++

# OpenMP-offload linker flag
LDFLAGS_OPENMP = $(LDFLAGS)

# CUDA compiler flags for C++ source file
NVCC_CXXFLAGS += -O3 -g -Wall -Wextra -pedantic

# Linker flag for CUDA device compilation (openMP-offload)
NVCC_LDFLAGS += -O3 -g -Wall -Wextra -pedantic

# Dependencies for CUDA compilation
CUDA_DEPS = $(DEPS)

# OpenMP-offload compilation for C++ source file
OPENMP_CXX=$(CXX) $(OPENMP_CXXFLAGS) $< -o $@

# CUDA compilation for C++ source file
NVCC=$(NVCC) $(NVCC_CXXFLAGS) $< -o $@.cu

# Makefile for cuda compilation
cuda: $(TARGET).cu

$(TARGET): $(TARGET).cu
    $(NVCC) $(NVCC_LDFLAGS) -c $(CUDA_DEPS)

$(OBJS): $(SRC:.c=.o)
    $(CXX) $(CCFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
    $(OPENMP_CXX) $(LDFLAGS_OPENMP) -fopenmp-offload $(DEPS)

# Clean target
clean:
    rm -rf *.cu *.o *.so *.exe

# Clean CUDA object file
cuda_clean:
    rm -rf $(TARGET).cu

# Clean all object files and executables
all: clean cuda_clean
