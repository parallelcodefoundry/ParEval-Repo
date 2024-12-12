# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offloading for parallel execution on a CPU.

## Prerequisites

OpenMP must be installed and supported by your compiler.  You'll need a compiler that supports OpenMP offloading (e.g., GCC, Clang).

## Build

To build microXOR, use `make`.
