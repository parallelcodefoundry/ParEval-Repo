# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offload for GPU execution.

## Prerequisites

OpenMP with offload support must be installed.

## Build

To build microXOR, use `make` with `CXX_COMPILER` set to the correct compiler and `CUDA_ARCH` set to the appropriate target GPU architecture. For example, the following will build microXOR for an NVIDIA GPU with compute capability 80 using the Clang compiler:
