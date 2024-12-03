#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= g++
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
TABLE       ?= no
SM_VERSION  ?= 80 # Updated for OpenMP offloading

#===============================================================================
# Program name & source code list
#===============================================================================

program = SimpleMOC-kernel

# Changed .cu to .cpp
source = \
main.cpp \
kernel.cpp \
init.cpp \
io.cpp

obj = $(source:.cpp=.o)

#===============================================================================
# Sets Flags
#===============================================================================

# Regular g++ Compiler
CC = g++

# Standard Flags
override CFLAGS += -fopenmp -foffload=-lm -foffload=-lcurand -Iinclude  #-I/usr/local/cuda/include  #Removed CUDA include, adjust as needed for your system
#Add necessary OpenMP and offloading flags here. Adjust paths as needed.

# Linker Flags
LDFLAGS = -lm -lcurand # -lcudart #Removed CUDA libraries, adjust as needed.

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
	CFLAGS += -O3 -ffast-math
endif

# Exponential Table Build
ifeq ($(TABLE), yes)
	CFLAGS += -DTABLE
endif

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) SimpleMOC-kernel_header.h
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) SimpleMOC-kernel_header.h

run:
	./$(program)