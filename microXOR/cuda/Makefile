# Makefile for CUDA microXOR

NVCC ?= nvcc
CUDA_ARCH ?= sm_80
NVCCFLAGS ?= -O3 -arch=$(CUDA_ARCH) -I. -std=c++17
LDFLAGS ?= -lcudart

CUFILES = microXOR.cu main.cu
DEPS = microXOR.cuh
OBJS = $(CUFILES:.cu=.o)
EXE = microXOR

$(EXE): $(OBJS)
	$(NVCC) $(NVCCFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

%.o: %.cu $(DEPS)
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean
