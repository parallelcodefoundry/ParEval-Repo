#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= llvm
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
CUDA_ARCH  ?= 80

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
CFLAGS := -fopenmp -foffload=-march=native -foffload=-mllvm -foffload=-mattr=+fp64

# Linker Flags
LDFLAGS = -lm

# LLVM Compiler
ifeq ($(COMPILER),llvm)
  CC = clang
  CFLAGS += -O3 -std=c++17 #Using C++17 for compatibility with std::chrono
  LDFLAGS += -L$(shell $(CC) -print-file-name=libomp) -Wl,-rpath,$(shell $(CC) -print-file-name=libomp)
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

# CUDA Offload Flags
ifeq ($(COMPILER),llvm)
  CFLAGS += -fopenmp-targets=nvptx64-nvidia-cuda11.8 --cuda-gpu-arch=$(CUDA_ARCH)
endif


#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) XSbench_header.h Makefile
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.c XSbench_header.h Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.h

run:
	./$(program)