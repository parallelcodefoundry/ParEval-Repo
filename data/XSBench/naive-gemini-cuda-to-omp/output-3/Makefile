#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= gcc
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
OFFLOAD     ?= yes

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
ifeq ($(COMPILER),gcc)
  CC = g++
  CFLAGS += -Wall -Wextra -fopenmp -std=c++14
  ifeq ($(OFFLOAD),yes)
    CFLAGS += -foffload=nvptx-none
  endif
endif

# Debug Flags
ifeq ($(DEBUG),yes)
  CFLAGS += -g -G
  LDFLAGS  += -g -G
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
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cu XSbench_header.cuh Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.cuh

run:
	./$(program)