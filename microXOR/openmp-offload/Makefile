# Makefile for OpenMP Offload microXOR

CXX_COMPILER ?= clang++
CUDA_ARCH ?= sm_80
CXXFLAGS ?= -O3 -I. -std=c++17 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -Xopenmp-target -march=$(CUDA_ARCH)
LDFLAGS ?= -lm

CPPFILES = microXOR.cpp main.cpp
DEPS = microXOR.hpp
OBJS = $(CPPFILES:.cpp=.o)
EXE = microXOR

$(EXE): $(OBJS)
	$(CXX_COMPILER) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

%.o: %.cpp $(DEPS)
	$(CXX_COMPILER) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean
