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

# Standard Flags
override CFLAGS += -foffload=-march=native -foffload=-mllvm -foffload=-fno-vectorize -fopenmp -I. -L$(CLANG_LIB_DIR)

# Linker Flags
LDFLAGS = -lm -lcurand

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
	CFLAGS += -O3 -ffast-math
endif

# Exponential Table Build
ifeq ($(TABLE), yes)
	CFLAGS += -DTABLE
endif

# Add extra flags for the offloading target (replace with your specific target flags if needed)
ifeq ($(SM_VERSION),80)
  CFLAGS += -foffload=nvptx64-nvidia-cuda -foffload=-march=sm_$(SM_VERSION)
endif


#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) SimpleMOC-kernel_header.h
	$(COMPILER) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.c
	$(COMPILER) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) SimpleMOC-kernel_header.h

run:
	./$(program)