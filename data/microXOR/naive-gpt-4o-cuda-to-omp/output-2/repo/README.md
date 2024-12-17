# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offloading for GPU execution.

## Prerequisites

A compiler with OpenMP 4.5 or later support for offloading to GPUs must be installed. Examples include GCC 9+ or Clang 10+.

## Build

To build microXOR, use `make`. Ensure your compiler supports OpenMP offloading and is correctly configured for your GPU.
