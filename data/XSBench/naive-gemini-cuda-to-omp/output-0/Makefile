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
CFLAGS := 

# Linker Flags
LDFLAGS = -lm

# GCC Compiler
ifeq ($(COMPILER),g++)
  CC = g++
  CFLAGS += -Wall -Wextra -fopenmp -foffload=nvptx-none
  # Optimization Flags
  ifeq ($(OPTIMIZE),yes)
    CFLAGS += -O3
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

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) XSbench_header.h Makefile
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cpp XSbench_header.h Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.h

run:
	./$(program)