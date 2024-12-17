#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= clang
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
TABLE       ?= no
SM_VERSION  ?= 80
NVHPC_PATH  ?= /opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/

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

# Regular compiler
ifeq ($(COMPILER),clang)
  CC = clang
endif

# Standard Flags
override CFLAGS += -fopenmp-targets=nvptx64-nvidia-cuda=10.2 -I/usr/local/cuda/include -L$(NVHPC_PATH) -lnvhpcomp -std=c++11

# Linker Flags
LDFLAGS = $(CFLAGS)

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
	CFLAGS += -O3 --use_fast_math
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


# NVPTX compilation target
NVPTX_TARGET = nvptx64-nvidia-cuda=10.2

#===============================================================================
# Compilation for NVIDIA GPU
#===============================================================================

nvidia: $(obj)
	$(CC) -fopenmp-targets=$(NVPTX_TARGET) $(CFLAGS) $(obj) -o SimpleMOC-kernel -lcuda -lnvrtc