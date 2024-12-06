# Makefile for XSBench (OpenMP-Offload version)

# Compiler flags
CFLAGS=-O3 -fopenmp -g -std=c++11
GPUFlags=-g

# Program name and input file
PROG=XSbench
INPUT=input.txt

# OpenCL code directory
CODE_DIR=xsbench_code

# CUDA/GPU executable output directory
BIN_DIR=xsbench_bin

# Build targets
XSbench: $(BIN_DIR)/$(PROG).exe

$(BIN_DIR)/$(PROG).exe: $(CODE_DIR)/$(PROG).cc $(CODE_DIR)/XSBench.h $(CODE_DIR)/Inputs.h $(CODE_DIR)/Profile.h
	@mkdir -p $(BIN_DIR)
	g++ $(CFLAGS) -o $@ $^

$(CODE_DIR)/$(PROG).cc: $(CODE_DIR)/$(PROG).cpp
	@mkdir -p $(CODE_DIR)
	g++ $(CFLAGS) -c -fopenmp --target=nvcc $< -o $@

# Install OpenCL and CUDA code to correct directories
$(CODE_DIR):
	cp -r /path/to/correct/opencl/code/*.so* $(CODE_DIR)/
	cp -r /path/to/correct/nvidia/gpu_code/*.cu $(CODE_DIR)/

$(BIN_DIR):
 cp -r /path/to/correct/nvidia/gpu_code/*.exe $(BIN_DIR)/

# Clean and rebuild all targets
.PHONY: clean
clean:
	@rm -rf $(BIN_DIR)
	@rm -f *.so*.*
	@rm -f *.exe
	@rm -rf $(CODE_DIR)

# Run XSbench with OpenMP-Offload execution model
run: XSbench
	./$(BIN_DIR)/$(PROG).exe $(INPUT) SM_VERSION=80
