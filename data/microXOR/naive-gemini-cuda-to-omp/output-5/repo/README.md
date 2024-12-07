# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a host with an accelerator device (e.g., GPU).

## Prerequisites

An OpenMP-capable compiler with offloading support (e.g., GCC, Clang) and an accelerator device are required.  The target architecture for offloading should be specified during compilation.

## Build

To build microXOR, use `make`.  The Makefile will attempt to automatically detect the appropriate offloading target, but you may need to adjust the `TARGET` variable if necessary. For example, to specify an NVIDIA GPU, you might need to use a compiler flag such as `-fopenmp-targets=nvptx64-nvidia-cuda`.
