#===============================================================================
# User Options
#===============================================================================

SM_VERSION  ?= 80

#===============================================================================
# Program name & source code list
#===============================================================================

program = SimpleMOC-kernel

source = \
main.c.o \
kernel.c.o \
init.c.o \
io.c.o

obj = $(source:.o=.c)

#===============================================================================
# Sets Flags
#===============================================================================

# Compiler for OpenMP and offload
CC = clang -fopenmp

# Standard Flags
override CFLAGS += -I/usr/local/cuda/include 
CXXFLAGS = -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/

# Linker Flags
LDFLAGS = -lm -lcurand

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj)
	$(CC) $(CFLAGS) $(CXXFLAGS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(source)

edit:
	vim -p $(obj)

run:
	./$(program)

#===============================================================================
# Flags for offload to NVIDIA GPU
#===============================================================================

NVCC = clang++-offload --fopenmp-offload -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/

CXXFLAGS_offload = $(CXXFLAGS)

# Compilation rule for OpenMP and offload
%.o_offload: %.c
	$(NVCC) $(CFLAGS) -c $< -o $@

#===============================================================================
# Offload Targets to Build
#===============================================================================

$(program)_offload: $(obj_offload)
	$(NVCC) $(CFLAGS) -o $@ $(LDFLAGS)

%.o_offload: %.c
	$(NVCC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(source)

edit:
	vim -p $(obj)

run:
	./$(program)_offload

#===============================================================================
# CUDA-related flags and compilation rules
#===============================================================================

CUDA_HOME = /usr/local/cuda

NVCC = clang++-offload --fopenmp-offload -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/

CXXFLAGS_offload = $(CXXFLAGS)

# Compilation rule for OpenMP and offload
%.o_offload: %.c
	$(NVCC) -D__CUDA_ARCH__=80 $(CFLAGS) -c $< -o $@

#===============================================================================
# Offload Targets to Build
#===============================================================================

$(program)_offload: $(obj_offload)
	$(NVCC) -D__CUDA_ARCH__=80 $(CFLAGS) -o $@ $(LDFLAGS)

%.o_offload: %.c
	$(NVCC) -D__CUDA_ARCH__=80 $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program)_offload

edit:
	vim -p $(obj)