CUDA_ARCH ?= 80
COMPILER ?= llvm

CC = ${COMPILER} -fopenmp
CXX = ${CC}
LD = ${CC}

CFLAGS = -Wall -Wextra -O3 -arch=$(CUDA_ARCH)
LDFLAGS = -lm
OPENMP_CFLAGS = -fopenmp-targets=nvptx64-nvvm34.9

program = XSBench

source = \
Main.cu \
io.cu \
Simulation.cu \
GridInit.cu \
XSutils.cu \
Materials.cu

obj = $(source:.cu=.o)

$(program): $(obj) XSbench_header.h Makefile
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.c XSbench_header.h Makefile
	$(CC) $(CFLAGS) $(OPENMP_CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) XSbench_header.h

run:
	./$(program)