# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offload for GPU execution.

## Prerequisites

A compiler with OpenMP Offload support must be installed. This typically includes compilers like GCC (version 9 or later) or Clang (version 10 or later) with OpenMP support.

## Build

To build microXOR, use `make`, setting `OMP_TARGET` as appropriate for your system. For example, the following will build microXOR for an NVIDIA GPU:
