# User Options
COMPILER ?= clang
OPTIMIZE ?= yes
DEBUG ?= no
PROFILE ?= no
CUDA_ARCH ?=

# Program name & source code list
program = XSBench

source = \
Main.cu \
io.cu \
Simulation.cu \
GridInit.cu \
XSutils.cu \
Materials.cu

obj = $(source:.cu=.o)

# Sets Flags
CFLAGS := 
LDFLAGS = -lm

# LLVM Compiler
ifeq ($(COMPILER),clang)
  CC = clang++-offload
  CXX = clang++
  CFLAGS += -Xcudarc --cuda-generate-code-for-all-gpus
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

# CUDA Architecture
ifneq ($(CUDA_ARCH),)
  CFLAGS += --cuda-gencode=$(CUDA_ARCH)
endif

# Targets to Build
$(program): $(obj) XSbench_header.cuh Makefile
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cu XSbench_header.cuh Makefile
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.cuh

run:
	./$(program)