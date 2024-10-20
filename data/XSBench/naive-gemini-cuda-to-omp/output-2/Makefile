#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= g++
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
TARGET      ?= x86_64-unknown-linux-gnu

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

# G++ Compiler
ifeq ($(COMPILER),g++)
  CC = g++
  CFLAGS += -Wall -Wextra -fopenmp -foffload=$(TARGET) -std=c++11
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