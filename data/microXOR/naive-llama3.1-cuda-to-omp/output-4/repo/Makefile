# Makefile for OpenMP microXOR

CXX ?= g++
SM_VERSION ?= $(SM_VERSION)
CXX_OFFLOAD_FLAGS ?= -fopenmp-targets=nvptx64-nvidia-cuda -fopenmp-target-exec=omp- target-device=nvidia,cuda=$(SM_VERSION)

SRC = src
INC = include
OBJ = obj

OBJS = $(foreach file,$(wildcard $(SRC)/*.cpp),$(file:$(SRC)/%.cpp=$(OBJ)/%.o))

EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX) -fopenmp -o $@ $^ $(CXX_OFFLOAD_FLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CXX) -c -fopenmp $< -o $@

# Cleanup
clean:
	rm -f $(EXE) $(OBJS)