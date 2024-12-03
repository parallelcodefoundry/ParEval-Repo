#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= icpc
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
main.cpp \
kernel.cpp \
init.cpp \
io.cpp

obj = $(source:.cpp=.o)

#===============================================================================
# Sets Flags
#===============================================================================

# Intel compiler
CC = icpc

# Standard Flags
override CFLAGS += -fPIC -foffload=$(TARGET) -I/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/include -std=c++11 -O3 -openmp -DOPENMP_OFFLOAD

# Linker Flags
LDFLAGS = -lm -L/opt/nvidia/hpc_sdk/Linux_x86_64/23.9/math_libs/12.2/targets/x86_64-linux/lib -lcurand

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

# Target specific flags (compute capability 80)
ifeq ($(SM_VERSION),80)
  CFLAGS += -fsycl -fsycl-targets=nvidia:compute_80 -Wno-sycl
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