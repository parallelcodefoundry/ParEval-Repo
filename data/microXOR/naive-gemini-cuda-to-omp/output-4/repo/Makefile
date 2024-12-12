# Makefile for OpenMP offloading microXOR

CXX ?= clang++
CXXFLAGS ?= -std=c++17 -fopenmp -foffload=-march=sm_$(SM_VERSION) -Iinclude
LDFLAGS ?= -fopenmp -lcudart #Note: -lcudart may not be needed depending on your compiler

ifeq ($(DEBUG), yes)
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O3
endif

SRC = src
INC = include
OBJ = obj
EXE = microXOR.exe

CUFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.hpp
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