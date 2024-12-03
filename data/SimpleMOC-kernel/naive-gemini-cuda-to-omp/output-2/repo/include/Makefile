#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= g++
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
TABLE       ?= no
SM_VERSION  ?= 80 # Set to your target SM version

#===============================================================================
# Program name & source code list
#===============================================================================

program = SimpleMOC-kernel

source = \
main.cpp \
kernel.cpp \
init.cpp \
io.cpp

obj = $(source:.cpp=.o)

#===============================================================================
# Sets Flags
#===============================================================================

# Standard Flags
override CFLAGS += -I. -fopenmp -foffload=-march=sm_$(SM_VERSION) -L$(CUDA_LIBS)

# Linker Flags
LDFLAGS = -lm -lcurand #Adjust curand library path if necessary

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

# CUDA Libraries Path (adapt as needed)
CUDA_LIBS ?= /opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib/


#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) SimpleMOC-kernel_header.h
	$(COMPILER) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(COMPILER) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) SimpleMOC-kernel_header.h

run:
	./$(program)