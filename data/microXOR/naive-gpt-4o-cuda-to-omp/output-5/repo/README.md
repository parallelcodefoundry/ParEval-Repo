# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offloading for GPU execution.

## Prerequisites

A compiler with OpenMP Offloading support must be installed. This typically includes compilers like GCC or Clang with OpenMP support.

## Build

To build microXOR, use `make`, setting the appropriate compiler and flags for OpenMP Offloading. For example, the following will build microXOR using GCC:
