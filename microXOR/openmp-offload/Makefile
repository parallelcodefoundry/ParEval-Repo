# Makefile for OpenMP Offload microXOR

CXX ?= clang++
CXXFLAGS ?= -O3 -I. -std=c++17 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -Xopenmp-target -march=sm_75
LDFLAGS ?= -lm

CPPFILES = microXOR.cpp main.cpp
DEPS = microXOR.hpp
OBJS = $(CPPFILES:.cpp=.o)
EXE = microXOR

$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean
