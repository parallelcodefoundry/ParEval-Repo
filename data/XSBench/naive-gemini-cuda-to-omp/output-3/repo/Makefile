#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= gcc
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
SM_VERSION  ?= 80

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
CFLAGS := -fopenmp -foffload=-lm

# Linker Flags
LDFLAGS = -lm

# NVIDIA Compiler flags for offloading
NVCCFLAGS = -foffload=-march=sm_$(SM_VERSION)


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

$(program): $(obj) XSbench_header.h XSbench_shared_header.h Makefile
	$(COMPILER) $(CFLAGS) $(NVCCFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cpp XSbench_header.h XSbench_shared_header.h Makefile
	$(COMPILER) $(CFLAGS) $(NVCCFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.h XSbench_shared_header.h

run:
	./$(program)