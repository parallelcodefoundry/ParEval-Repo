# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a supported accelerator (e.g., GPU).

## Prerequisites

An OpenMP-enabled compiler with offloading support (e.g., GCC, Clang) and a compatible accelerator are required.  Ensure your system is properly configured for OpenMP offloading.

## Build

To build microXOR, use `make`.  No specific architecture flags are needed as OpenMP handles target selection.
