#===============================================================================
# User Options
#===============================================================================

SM_VERSION  ?= 80
CXX          ?= clang
LIBS         ?= -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/

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

# OpenMP Compiler Flags
CXXFLAGS += -fopenmp

# Standard GCC Flags
override CXXFLAGS += -I/usr/local/cuda/include -arch sm_$(SM_VERSION) 

# Linker Flags
LDFLAGS = -lm -lcurand -lomp

# Profiling Flags
CXXFLAGS += -pg

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj)
	$(CXX) $(CXXFLAGS) $(obj) -o $@ $(LIBS) $(LDFLAGS)

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) SimpleMOC-kernel_header.h

run:
	./$(program)


# Offloading flags for CUDA
ifeq ($(shell uname), Linux)
    OFFLOAD_DIR := /opt/nvidia/hpc_sdk/Linux_x86_64/23.9/offload/12.2/targets/x86_64-linux/bin/
else ifeq ($(shell uname | grep -iq win), yes)
    OFFLOAD_DIR := "C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.x\bin"
endif

OFFLOAD_FLAGS = -Xoffload-arch=$(SM_VERSION) -Xoffload-target=gpu -g