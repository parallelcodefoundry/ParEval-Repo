# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offloading for parallel execution on a CPU.

## Prerequisites

A C++ compiler with OpenMP 5.0 support is required.  You may need to specify compiler flags to enable OpenMP offloading (e.g., `-fopenmp` for GCC or Clang).

## Build

To build microXOR, use `make`.
