# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP for offloading to a GPU.

## Prerequisites

OpenMP with offloading support must be installed. Ensure your compiler supports OpenMP offloading (e.g., GCC with OpenMP offloading).

## Build

To build microXOR, use `make`, setting `OMP_TARGET` as appropriate for your system. For example, the following will build microXOR for an OpenMP-compatible GPU.
