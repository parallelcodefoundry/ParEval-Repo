# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for GPU execution.

## Prerequisites

A compiler with OpenMP offloading support must be installed. This typically includes compilers like GCC or Clang with OpenMP support.

## Build

To build microXOR, use `make`, ensuring your compiler supports OpenMP offloading. You may need to specify the target architecture for your system. For example, the following will build microXOR for an NVIDIA GPU:
