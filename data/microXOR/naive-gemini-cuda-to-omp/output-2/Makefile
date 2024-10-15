# Makefile for OpenMP offload microXOR

CXX_COMPILER ?= g++
OMP_FLAGS ?= -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda
CXXFLAGS ?= -O3 -Iinclude $(OMP_FLAGS) -std=c++17
LDFLAGS ?= -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.1/cuda/12.0/lib64 -L/global/common/software/nersc/pe/gpu/llvm/17.0.6/lib -lcudart_static -ldl -lrt -pthread

SRC = src
INC = include
OBJ = obj
CUFILES = $(SRC)/microXOR.cu $(SRC)/main.cu
DEPS = $(INC)/microXOR.cuh
OBJS = $(CUFILES:$(SRC)/%.cu=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX_COMPILER) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cu $(DEPS) | obj
	$(CXX_COMPILER) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean