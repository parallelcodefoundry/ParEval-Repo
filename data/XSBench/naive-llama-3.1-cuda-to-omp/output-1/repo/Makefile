# Translated from original Makefile

# Default target
all: xsbench_shared.o xsbench_kernel.o xsbench_offload.o
    nvcc -o xsbench -O3 --offload-architecture=compute_80 xsbench_shared.o xsbench_kernel.o xsbench_offload.o $(CUDAFLAGS)

# Rule for building xsbench_shared.o
xsbench_shared.o: xsbench_shared.c xsbench_shared.h
    ${COMPILER} -c -O3 -o $@ $<

# Rule for building xsbench_kernel.o
xsbench_kernel.o: xsbench_kernel.c xsbench_shared.h
    ${COMPILER} -c -O3 -o $@ $<

# Rule for building xsbench_offload.o
xsbench_offload.o: xsbench_offload.c xsbench_shared.h
    ${COMPILER} -fopenmp-offload -c -O3 -o $@ $<