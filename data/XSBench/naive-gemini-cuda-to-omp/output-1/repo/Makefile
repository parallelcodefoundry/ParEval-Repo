#===============================================================================
# User Options
#===============================================================================

COMPILER    ?= clang
OPTIMIZE    ?= yes
DEBUG       ?= no
PROFILE     ?= no
SM_VERSION  ?= 80

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
CFLAGS := -fopenmp -foffload=-lm=-m64

# Linker Flags
LDFLAGS = -lm

# Clang Compiler with OpenMP offloading
ifeq ($(COMPILER),clang)
  CC = clang
  CFLAGS += -Wall -Wextra -O3 -march=native -fopenmp -foffload=${CFLAGS}
  # Add architecture specific flags if needed for target offloading, e.g., -foffload=-march=sm_$(SM_VERSION) for NVidia GPUs
  LDFLAGS += -fopenmp
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