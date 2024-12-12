#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= llvm
CUDA_ARCH   ?= 80
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no

#===============================================================================
# Program name & source code list
#===============================================================================

program = XSBench

source = \
Main.c \
io.c \
Simulation.c \
GridInit.c \
XSutils.c \
Materials.c

obj = $(source:.c=.o)

#===============================================================================
# Sets Flags
#===============================================================================

# Standard Flags
CFLAGS := -fopenmp -foffload=-fopenmp -foffload=-lm

# Linker Flags
LDFLAGS = -lm

# Compiler selection
ifeq ($(COMPILER),llvm)
  CC = clang
  CFLAGS += -O3 -march=native -fopenmp-targets=nvptx64-nvidia-cuda
  LDFLAGS += -fopenmp-targets=nvptx64-nvidia-cuda
  # Add CUDA architecture flag if provided
  ifeq ($(CUDA_ARCH),)
  else
    CFLAGS += -mcuda-arch=$(CUDA_ARCH)
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

$(program): $(obj) XSbench_header.h XSbench_shared_header.h Makefile
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.c XSbench_header.h XSbench_shared_header.h Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.h XSbench_shared_header.h

run:
	./$(program)