# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offloading for GPU execution.

## Prerequisites

A compiler with OpenMP 4.5 or later support and a compatible GPU is required. Examples include GCC 9+ or Clang 10+ with appropriate GPU support.

## Build

To build microXOR, use `make`. Ensure your compiler is set up to support OpenMP offloading. You may need to adjust the `Makefile` to specify the correct compiler and flags for your system.
