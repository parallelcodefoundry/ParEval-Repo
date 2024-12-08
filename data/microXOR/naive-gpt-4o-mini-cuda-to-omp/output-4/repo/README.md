# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offload for GPU execution.

## Prerequisites

OpenMP with offloading support must be installed. Ensure that your compiler supports OpenMP offloading to your target GPU.

## Build

To build microXOR, use `make`, setting `OMP_TARGET` as appropriate for your system. For example, the following will build microXOR for a compatible GPU.
