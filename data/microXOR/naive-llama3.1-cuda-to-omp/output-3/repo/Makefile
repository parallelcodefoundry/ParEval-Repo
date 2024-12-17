# Makefile for OpenMP microXOR offload

SM_VERSION ?= $(CXX_COMPILER)
CXX_COMPILER ?= g++
OFFLOAD_ARCH ?= sm_80
SRC = src
INC = include
OBJ = obj
OFFLOADFLAGS ?= -fopenmp-targets=nvptx64-nvidia-cuda --cuda-gpu-arch=$(OFFLOAD_ARCH) -std=c++17
ifneq ($(DEBUG), yes)
    OFFLOADFLAGS += -O3
else
    OFFLOADFLAGS += -g -G
endif
LDFLAGS ?= -fopenmp

CUFILES = $(SRC)/microXOR.cpp $(SRC)/main.cpp
DEPS = $(INC)/microXOR.hpp
OBJS = $(CUFILES:$(SRC)/%.cpp=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
    $(CXX_COMPILER) $(OFFLOADFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cpp $(DEPS) | obj
    $(CXX_COMPILER) $(OFFLOADFLAGS) -c $< -o $@

obj:
    mkdir -p $(OBJ)

clean:
    rm -f $(OBJS) $(EXE)

.PHONY: clean
