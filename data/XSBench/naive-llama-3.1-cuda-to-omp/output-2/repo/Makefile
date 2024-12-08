# Makefile for XSBench shared utilities

CXX = clang++
CUDA_CXX = nvcc
NVCC_ARCH = $(CUDA_ARCH)
COMPILE = COMPILER=$(COMPILER)

default: all

all: xsbench_shared.o

xsbench_shared.o: $(XSBECH_SHARED_HEADER_H) $(XSBECH_SHARED_SOURCE_FILE).c
    @echo "Compiling $<"
    $(CXX) -c -g3 -std=c++11 -O0 -Wall -Wno-deprecated-gpu-targets -fPIC -DUSE_OFFLOAD=$(CUDA_ARCH) \
        $(NVCC_CXXFLAGS) $(COMPILE) $(XSBECH_SHARED_SOURCE_FILE).c -o xsbench_shared.o

clean:
    rm -rf *.o