#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= nvidia
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
SM_VERSION  ?= 80

CXX         = clang++
LD          = clang++

# OpenMP flags
OPENMPFLAGS = -fopenmp -std=c++14

# Offload flag
OFFLOAD     = -fopenacc

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
CFLAGS := -O3

# OpenMP Compiler Flags
OMP_CXXFLAGS = $(OPENMPFLAGS) -arch=sm_$(SM_VERSION)
OMP_LDFLAGS  = $(OFFLOAD)

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

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) XSbench_header.cuh Makefile
	$(CXX) $(OMP_CXXFLAGS) $(CFLAGS) $(obj) -o $@ $(OMP_LDFLAGS)

%.o: %.cu XSbench_header.cuh Makefile
	$(CXX) $(OMP_CXXFLAGS) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.cuh

run:
	./$(program)


io.cu:
#include "XSbench_header.cuh"

// Prints program logo
void logo(int version)
{
	border_print();
	printf(
	"                   __   __ ___________                 _                        \n"
	"                   \\ \\ / //  ___| ___ \\               | |                       \n"
	"                    \\ V / \\ `--.| |_/ / ___ _ __   ___| |__                     \n"
	"                    /   \\  `--. \\ ___ \\/ _ \\ '_ \\ / __| '_ \\                    \n"
	"                   / /^\\ \\/\\__/ / |_/ /  __/ | | | (__| | | |                   \n"
	"                   \\/   \\/\\____/\\____/ \\___|_| |_|\\___|_| |_|                   \n\n"
	       );
	border_print();
	center_print("Developed at Argonne National Laboratory", 79);
	char v[100];
	sprintf(v, "Version: %d", version);
	center_print(v, 79);
	border_print();
}

// ...
