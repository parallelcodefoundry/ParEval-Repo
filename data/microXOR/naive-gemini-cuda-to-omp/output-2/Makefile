# Makefile for OpenMP Offload microXOR

CXX ?= g++
OMPFLAGS ?= -fopenmp -Xpreprocessor -fopenmp-targets=nvptx-none
CXXFLAGS ?= -O3 -std=c++17 $(OMPFLAGS)
LDFLAGS ?= -fopenmp

SRC = src
INC = include
OBJ = obj
EXE = microXOR.exe

CUFILES = $(SRC)/microXOR.cu $(SRC)/main.cu
DEPS = $(INC)/microXOR.cuh
OBJS = $(CUFILES:$(SRC)/%.cu=$(OBJ)/%.o)

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cu $(DEPS) | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean