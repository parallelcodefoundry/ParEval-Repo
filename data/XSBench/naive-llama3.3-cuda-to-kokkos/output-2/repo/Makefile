#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= g++
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no

#===============================================================================
# Program name & source code list
#===============================================================================

program = XSBench

source = \
Main.cpp \
io.cpp \
Simulation.cpp \
GridInit.cpp \
XSutils.cpp \
Materials.cpp

obj = $(source:.cpp=.o)

#===============================================================================
# Sets Flags
#===============================================================================

# Standard Flags
CFLAGS := -std=c++14

# Linker Flags
LDFLAGS = -lm

# Compiler
CC = $(COMPILER)

# Debug Flags
ifeq ($(DEBUG),yes)
  CFLAGS += -g -O0
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

# Kokkos Flags
KOKKOS_PATH ?= /path/to/kokkos
CFLAGS += -I$(KOKKOS_PATH)/core/src
LDFLAGS += -L$(KOKKOS_PATH)/lib -lkokkoscore

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) XSbench_header.hpp Makefile
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cpp XSbench_header.hpp Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.hpp

run:
	./$(program)