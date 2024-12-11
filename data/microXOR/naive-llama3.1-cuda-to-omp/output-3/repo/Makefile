CXX = clang++
CXXFLAGS = -std=c++11 -fopenmp-targets=nvptx64-nvidia-cuda -fopenmp-compile -D__NVCC_DIAG_PRAGMA_SUPPORT__
SM_VERSION ?= 80
TARGET_ARCH ?= nvcc

ifeq ($(shell uname), Darwin)
    CXXFLAGS += -I /usr/local/cuda/include
else
    # Linux or other platforms
endif

# Add OpenMP include path
CXXFLAGS += -I/usr/include/openmp

# Set up the offload compiler and flags for different targets
OPENMPIR = $(shell ompccgetdevicecount)

ifeq ($(OPENMPIR), 1)
    CXXFLAGS += -fopenmp-targets=ptx64-nvidia-cuda -fopenmp-compile
endif

CXXFLAGS += -O3 -g -D __CUDA_HOST_API__ -DMICROXOR_KERNEL -mcu=$(SM_VERSION) -arch sm_$(SM_VERSION)

ifneq ($(shell which nvcc),)
    CXX = nvcc
else
    # Use OpenMP-offload with a different compiler (e.g., clang++)
endif

all: microxor

microxor: $(wildcard *.cpp $(wildcard *.hpp))
        $(CXX) -o $@ $^ $(CXXFLAGS)

.PHONY: clean
clean:
        rm -f *.o microxor