# microXOR: XOR stencil offload benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

## Prerequisites

Offloading to OpenMP requires a C++ compiler that supports OpenMP and a system with multiple CPU cores. This can be done by compiling with the `-fopenmp` flag.

## Build

To build microXOR, use the following command: