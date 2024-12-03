# Makefile for OpenMP offload microXOR

CXX_COMPILER ?= g++
OMP_FLAGS ?= -fopenmp -Xopenmp-target -march=native
SRC = src
INC = include
OBJ = obj
CXXFLAGS ?= -O3 -I$(INC) -std=c++17 $(OMP_FLAGS)
LDFLAGS ?= -fopenmp

CUFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.cuh
OBJS = $(CUFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX_COMPILER) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CXX_COMPILER) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean