# Makefile for OpenMP-offload microXOR

SM_VERSION ?= 80
CXX_COMPILER ?= g++
OFFLOAD_CXX ?= offload
OFFLOAD_FLAGS ?= -fopenmp-targets=nvptx64-nvidia-cuda --cuda-device
ifneq ($(DEBUG), yes)
	CXXFLAGS += -O3
else
	CXXFLAGS += -g -G
endif

SRC = src
INC = include
OBJ = obj
CUFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.hpp
OBJS = $(CUFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX_COMPILER) -fopenmp $(OFFLOAD_FLAGS) $(OBJS) -o $(EXE)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CXX_COMPILER) -fopenmp $(OFFLOAD_FLAGS) $(CXXFLAGS) $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean

# Example usage:
# make SM_VERSION=80 CXX_COMPILER=clang++ CXX=g++