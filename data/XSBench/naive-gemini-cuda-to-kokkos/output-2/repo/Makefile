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
CFLAGS := -std=c++17 -I$(KOKKOS_ROOT)/include

# Linker Flags
LDFLAGS = -lm -L$(KOKKOS_ROOT)/lib -lkokkos_core

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

# Kokkos Specific Flags (Adjust as needed based on your Kokkos configuration)
KOKKOS_FLAGS =  # Add any necessary Kokkos-specific flags here, e.g., for specific backends


#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) XSbench_header.hpp Makefile
	$(COMPILER) $(CFLAGS) $(KOKKOS_FLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cpp XSbench_header.hpp Makefile
	$(COMPILER) $(CFLAGS) $(KOKKOS_FLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.hpp

run:
	./$(program)