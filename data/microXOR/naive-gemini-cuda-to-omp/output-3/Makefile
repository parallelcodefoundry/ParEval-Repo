# Makefile for OpenMP offload microXOR

CXX_COMPILER ?= g++
OMP_FLAGS ?= -fopenmp -march=native -O3
CXXFLAGS ?= $(OMP_FLAGS) -Iinclude -std=c++17
LDFLAGS ?= -fopenmp

CUFILES = src/microXOR.cpp src/main.cpp
DEPS = include/microXOR.cuh
OBJS = $(CUFILES:src/%.cpp=obj/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CXX_COMPILER) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: src/%.cpp $(DEPS) | obj
	$(CXX_COMPILER) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p obj

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean