# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offloading for GPU execution.

## Prerequisites

A compiler with OpenMP 4.5 or later support and an appropriate GPU runtime must be installed.

## Build

To build microXOR, use `make`. Ensure your compiler supports OpenMP offloading. For example, if using `clang`, you might need to specify the target architecture.
