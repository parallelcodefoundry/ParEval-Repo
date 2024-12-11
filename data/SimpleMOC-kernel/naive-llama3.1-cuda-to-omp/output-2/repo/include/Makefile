# Makefile for openmp-offload execution model

SM_VERSION ?= 80
CXX = clang
CUDA_DIR = /usr/local/cuda-11.x

ifeq ($(shell uname -s), Linux)
    CFLAGS += -L$(CUDA_DIR)/lib64 -lcudart -lcusolver -lculibos -lnvrtc_static 
    CXX = $(CUDA_DIR)/bin/nvcc
else ifeq ($(shell uname -s), Darwin)
    CFLAGS += -L/opt/cuda-11.x/targets/x86_64-darwin/lib/stubs -lcudart -lcusolver -lculibos -lnvrtc_static 
    CXX = $(CUDA_DIR)/bin/nvcc
endif

# OpenMP flags for offloading to GPU
OPENMP_FLAGS = -fopenmp-targets=nvidia-cuda --offload-architecture=sm_$(SM_VERSION)

CFLAGS += -O3 -std=c++11 -D__NVCC__ $(OPENMP_FLAGS)
LDFLAGS += -lcudart

OBJS = init.o main.o utils.o cuda_helper.o

%.o: %.c
    $(CXX) -c $< -o $@ $(CFLAGS)

init.o: init.c utils.h
main.o: main.c init.h utils.h
utils.o: utils.c
cuda_helper.o: cuda_helper.c

all: program.exe

program.exe: main.o init.o utils.o cuda_helper.o
    $(CXX) -o $@ $^ $(LDFLAGS)

clean:
    rm -f *.o *.exe