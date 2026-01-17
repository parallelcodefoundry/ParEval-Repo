# Makefile for CUDA nanoXOR

CUDA_ARCH ?= sm_80
NVCCFLAGS ?= -arch=$(CUDA_ARCH) -std=c++17
ifneq ($(DEBUG), yes)
	NVCCFLAGS += -O3
else
	NVCCFLAGS += -g -G
endif

nanoXOR.exe: nanoXOR.cu
	nvcc $(NVCCFLAGS) nanoXOR.cu -o nanoXOR.exe

clean:
	rm -f nanoXOR.exe

.PHONY: clean
