CC = clang++
CUDA_ARCH = $(shell nvidia-smi --query-gpu=gpu_architecture --id=0 | tr 'n' '\n')
CXXFLAGS=-g -Wall -std=c++11 -fopenmp

# Allow user to override these variables from the command line.
override COMPILER ?= clang++
override CUDA_ARCH ?= $(shell nvidia-smi --query-gpu=gpu_architecture --id=0 | tr 'n' '\n')
override SIMULATION_METHOD ?= 1
override NUM_ITERATIONS ?= 10

# The following lines are used to determine the correct set of compiler flags.
ifeq ($(COMPILER), clang++)
  # If using Clang, we can use OpenMP offload and CUDA as is. We will also add a flag
  # for OpenACC if it's available on this system.
  CXXFLAGS += -DUSE_OPENMP_OFFLOAD -DCUDA_ARCH=$(CUDA_ARCH)
endif

# Now make sure that the user has CUDA available, and set up the proper flags.
ifneq ($(shell which nvcc),)
    # We're using OpenACC. Set it up properly so we can use all of its features.
  CXXFLAGS += -DUSE_OPENACC
else
  # Otherwise, we're just going to use OpenMP offload and CUDA as is.
endif

# Now make sure that the user has a valid simulation method specified.
ifeq ($(SIMULATION_METHOD), 1)
    SIMULATION_METHOD = Monte Carlo
endif

ifeq ($(SIMULATION_METHOD), 2)
    SIMULATION_METHOD = Deterministic
endif

ifneq ($(filter 1 2,$(SIMULATION_METHOD)),)
    CXXFLAGS += -DSIMULATION_METHOD=$(SIMULATION_METHOD)
endif

# The following lines are used to compile and link the code.
all: $(EXECUTABLE)

%.o: %.c
    $(CC) $(CXXFLAGS) $< -o $@

$(EXECUTABLE): main.o xsbench_shared.o
    $(CC) $^ -o $@

run:
    ./$(EXECUTABLE) $(CUDA_ARCH) $(NUM_ITERATIONS)

clean:
    rm -f *.o $(EXECUTABLE)