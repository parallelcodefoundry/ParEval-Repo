# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a CPU.

## Prerequisites

A C++ compiler with OpenMP 5.0 support is required.  The compiler should support offloading to a target device (e.g., using `-fopenmp-targets=nvptx64-nvidia-cuda` for NVIDIA GPUs).

## Build

To build microXOR, use `make`.  The Makefile will automatically detect the target architecture if environment variables are set properly.

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads per block (which translates to the chunk size for OpenMP). For example, the following will run microXOR with a 1024x1024 input matrix and a chunk size of 32x32:
