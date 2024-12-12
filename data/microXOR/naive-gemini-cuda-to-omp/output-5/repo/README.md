# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offloading for GPU execution (if available).  It falls back to CPU execution if offloading is not supported.

## Prerequisites

An OpenMP-capable compiler with offloading support (e.g., g++ with -fopenmp) is required.  For GPU offloading, a compatible device (e.g., NVIDIA GPU with appropriate drivers) must be present.


## Build

To build microXOR, use `make`.
