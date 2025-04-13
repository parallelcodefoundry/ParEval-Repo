# Makefile for OpenMP Offload microXORh

CXX_COMPILER ?= clang++
SM_VERSION ?= sm_80
SRC = src
INC = include
OBJ = obj
CXXFLAGS ?= -I$(INC) -std=c++17 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -Xopenmp-target -march=$(SM_VERSION)
ifneq ($(DEBUG), yes)
	CXXFLAGS += -O3
else
	CXXFLAGS += -g
endif
LDFLAGS ?= -lm

CPPFILES = $(SRC)/main.cpp
DEPS = $(INC)/microXORh.hpp
OBJS = $(CPPFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
EXE = microXORh.exe

$(EXE): $(OBJS)
	$(CXX_COMPILER) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
	$(CXX_COMPILER) $(CXXFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean
