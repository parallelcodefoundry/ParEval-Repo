# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a CPU.

## Prerequisites

An OpenMP-capable compiler (e.g., GCC, Clang) with OpenMP offloading support is required.  The target device (e.g., a manycore CPU) must be accessible and configured correctly.


## Build

To build microXOR, use `make`.  No specific architecture flags are needed as OpenMP will automatically target the available hardware.
