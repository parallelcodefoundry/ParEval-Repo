# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offload for GPU execution.

## Prerequisites

A compiler with OpenMP Offload support must be installed. This typically includes compilers like GCC with OpenMP support or Clang.

## Build

To build microXOR, use `make`, ensuring your compiler is set up for OpenMP Offload. You may need to specify the target architecture for your system. For example, the following will build microXOR for an NVIDIA GPU:
