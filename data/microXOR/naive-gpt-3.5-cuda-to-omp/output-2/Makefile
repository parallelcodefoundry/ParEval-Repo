# Makefile for OpenMP + GPU Offload microXOR

CC = nvcc
CUDA_ARCH = sm_80
SRC = src
INC = include
OBJ = obj
CFLAGS = -O3 -arch=$(CUDA_ARCH) -I$(INC) -std=c++17
LDFLAGS = -lcudart

CUFILES = $(SRC)/microXOR.cu $(SRC)/main.cu
DEPS = $(INC)/microXOR.cuh
OBJS = $(CUFILES:$(SRC)/%.cu=$(OBJ)/%.o)
EXE = microXOR.exe

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -Xcompiler -fopenmp -o $(EXE) $(OBJS) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cu $(DEPS) | obj
	$(CC) $(CFLAGS) -Xcompiler -fopenmp -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean