# Makefile for CUDA microXOR

NVCC ?= nvcc
NVCCFLAGS ?= -O3 -arch=sm_75 -I. -std=c++17
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
