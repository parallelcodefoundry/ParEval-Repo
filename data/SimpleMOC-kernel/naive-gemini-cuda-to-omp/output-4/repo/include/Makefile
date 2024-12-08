#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= clang
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
TABLE       ?= no
SM_VERSION  ?= 80

#===============================================================================
# Program name & source code list
#===============================================================================

program = SimpleMOC-kernel

source = \
main.c \
kernel.c \
init.c \
io.c

obj = $(source:.c=.o)

#===============================================================================
# Sets Flags
#===============================================================================

# Regular clang Compiler
CC = clang++

# Standard Flags
override CFLAGS += -fopenmp -foffload=-lm -foffload=-lcurand -Iinclude -march=native  -Xopenmp-target -mavx2

# Add target specific flags
ifeq ($(findstring 80,$(SM_VERSION)),80)
  CFLAGS += -Xopenmp-target=nvptx80 -foffload=nvptx-none -L$(shell echo "/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/" | sed 's/\/$//')
endif

# Linker Flags


# Debug Flags
ifeq ($(DEBUG),yes)
  CFLAGS += -g
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
	$(CC) $(CFLAGS) $(obj) -o $@ 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) SimpleMOC-kernel_header.h

run:
	./$(program)