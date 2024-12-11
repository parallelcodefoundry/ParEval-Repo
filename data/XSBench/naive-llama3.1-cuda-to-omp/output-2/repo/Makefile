# User Options
CUDA_ARCH ?=
COMPILER    ?= llvm

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

# Sets Flags
CFLAGS := -Wall -Wextra
LDFLAGS = -lm

# NVIDIA Compiler
ifeq ($(COMPILER),llvm)
  CC = clang++
  CFLAGS += -g -O3 -std=c++14 -fopenmp-targets=nvptx64-nvidia-cuda=8.0 -DCUDA_ARCH=$(CUDA_ARCH)
endif

# Debug Flags
ifeq ($(DEBUG),yes)
  CFLAGS += -g
endif

# Profiling Flags
ifeq ($(PROFILE),yes)
  CFLAGS += -pg
endif

# Optimization Flags
ifeq ($(OPTIMIZE),yes)
  CFLAGS += -O3
endif

# Targets to Build
$(program): $(obj) XSbench_header.cuh Makefile
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cu XSbench_header.cuh Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.cuh

run:
	./$(program)