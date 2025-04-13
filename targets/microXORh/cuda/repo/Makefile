# Makefile for CUDA microXORh

NVCC ?= nvcc
CUDA_ARCH ?= sm_80
SRC = src
INC = include
OBJ = obj
NVCCFLAGS ?= -arch=$(CUDA_ARCH) -I$(INC) -std=c++17
ifneq ($(DEBUG), yes)
	NVCCFLAGS += -O3
else
	NVCCFLAGS += -g -G
endif
LDFLAGS ?= -lcudart

CUFILES = $(SRC)/main.cu
DEPS = $(INC)/microXORh.cuh
OBJS = $(CUFILES:$(SRC)/%.cu=$(OBJ)/%.o)
EXE = microXORh.exe

$(EXE): $(OBJS)
	$(NVCC) $(NVCCFLAGS) $(OBJS) -o $(EXE) $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.cu $(DEPS) | obj
	$(NVCC) $(NVCCFLAGS) -c $< -o $@

obj:
	mkdir -p $(OBJ)

clean:
	rm -f $(OBJS) $(EXE)

.PHONY: clean
