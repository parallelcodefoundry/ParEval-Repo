#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= clang
CUDA_ARCH   ?= 80
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
MPI         ?= no

#===============================================================================
# Program name & source code list
#===============================================================================

program = XSBench

source = \
Main.cu \
io.cu \
Simulation.cu \
GridInit.cu \
XSutils.cu \
Materials.cu

obj = $(source:.cu=.o)

#===============================================================================
# Sets Flags
#===============================================================================

# Standard Flags
CFLAGS := -std=c++14

# OpenMP Flags
CFLAGS += -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda=10.0

# Linker Flags
LDFLAGS = -lm -lopenomp

# Compiler
ifeq ($(COMPILER),clang)
  CC = clang++
endif
ifeq ($(COMPILER),gcc)
  CC = g++
endif
ifeq ($(COMPILER),nvcc)
  CC = nvcc
endif

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) XSbench_header.cuh Makefile
	$(CC) -O3 $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cu XSbench_header.cuh Makefile
	$(CC) -c $(CFLAGS) -fopenmp-targets=nvptx64-nvidia-cuda=10.0 $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.cuh

run:
	./$(program)