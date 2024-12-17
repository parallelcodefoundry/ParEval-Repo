# User Options
COMPILER ?= clang
CUDA_ARCH ?= 80
OPTIMIZE ?= yes
DEBUG    ?= no
PROFILE  ?= no

# Program name & source code list
program = XSBench

source = \
Main.cu \
io.cu \
Simulation.cu \
GridInit.cu \
XSutils.cu \
Materials.cu

obj = $(source:.cu=.o)

# Sets Flags for OpenMP offload
CFLAGS := -fopenmp -D__CUDA_ARCH__=$(CUDA_ARCH) -std=c++14

# Sets Flags for compiler choice (clang or other)
ifeq ($(COMPILER),clang)
  CC = clang++
else ifeq ($(COMPILER),gcc)
  CC = g++
endif

# Debug Flags
ifeq ($(DEBUG),yes)
  CFLAGS += -g -G
endif

# Profiling Flags
ifeq ($(PROFILE),yes)
  CFLAGS += -pg
endif

# Optimization Flags
ifeq ($(OPTIMIZE),yes)
  CFLAGS += -O3
endif

# Target to Build
$(program): $(obj) XSbench_header.cuh Makefile
	$(CC) $(CFLAGS) $(obj) -o $@ -fopenacc

%.o: %.cu XSbench_header.cuh Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.cuh

run:
	./$(program)


io.cu:
#include "XSbench_header.cuh"

// ...
