#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= g++
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
TARGET      ?= -fopenmp-targets=nvptx64-nvidia-cuda

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
CFLAGS := 

# Linker Flags
LDFLAGS = -lm

# GCC Compiler
ifeq ($(COMPILER),g++)
  CC = g++
  CFLAGS += -Wall -Wextra -std=c++14
endif

# Debug Flags
ifeq ($(DEBUG),yes)
  CFLAGS += -g
  LDFLAGS  += -g
endif

# Profiling Flags
ifeq ($(PROFILE),yes)
  CFLAGS += -pg
  LDFLAGS  += -pg
endif

# Optimization Flags
ifeq ($(OPTIMIZE),yes)
  CFLAGS += -O3
endif

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) XSbench_header.cuh Makefile
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS) $(TARGET)

%.o: %.cu XSbench_header.cuh Makefile
	$(CC) $(CFLAGS) -c $< -o $@ $(TARGET)

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.cuh

run:
	./$(program)