# Makefile for OpenMP + GPU Offload microXOR

CXX ?= g++
CXXFLAGS ?= -O3 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda --cuda-path=$(CUDA_HOME) -I$(INC) -std=c++17
LDFLAGS ?= -lm

SRC = src
INC = include
OBJ = obj
CXXFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.hpp
OBJS = $(CXXFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean