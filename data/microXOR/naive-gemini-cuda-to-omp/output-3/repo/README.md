# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for execution on accelerators.

## Prerequisites

An OpenMP-capable compiler with offloading support (e.g., GCC, Clang) and a compatible accelerator (e.g., Intel GPU, NVIDIA GPU) are required.  The necessary libraries for the target accelerator may need to be installed.

## Build

To build microXOR, use `make`.  The Makefile will automatically detect the appropriate compiler flags based on the target architecture.
