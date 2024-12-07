# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a CPU.

## Prerequisites

An OpenMP-capable compiler (like GCC or Clang) with OpenMP offloading support for your target device is required.

## Build

To build microXOR, use `make`.  No special flags are needed for specifying the target architecture as OpenMP handles target selection differently than CUDA.
