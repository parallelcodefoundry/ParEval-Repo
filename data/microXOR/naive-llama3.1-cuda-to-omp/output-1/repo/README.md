# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ and uses OpenMP with GPU offloading for parallelization on multiple devices (GPUs).

## Prerequisites

- A CPU and one or more NVIDIA GPUs
- CUDA toolkit and OpenMPI installed
- A compiler that supports OpenMP, such as GCC 5.1+ or Clang 3.7+

## Build

To build microXOR, use `cmake` to generate the Makefiles, then run `make`. You may need to set some environment variables depending on your system configuration.
