#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= gcc
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

# Standard Flags
override CFLAGS += -fopenmp -I/usr/local/cuda/include

# Linker Flags
LDFLAGS = -lm -lcudart -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/

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
	CFLAGS += -O3
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

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) SimpleMOC-kernel_header.h

run:
	./$(program)