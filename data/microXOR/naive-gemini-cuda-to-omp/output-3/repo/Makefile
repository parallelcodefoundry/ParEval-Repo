# Makefile for OpenMP offloading microXOR

CXX ?= g++
OMP_FLAGS ?= -fopenmp -foffload=-march=sm_$(SM_VERSION) -foffload
CXXFLAGS ?= -std=c++17 $(OMP_FLAGS) -Iinclude
LDFLAGS ?=

SRC = src
INC = include
OBJ = obj
EXE = microXOR.exe

CUFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.h
OBJS = $(CUFILES:$(SRC)/%.cpp=$(OBJ)/%.o)


$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean