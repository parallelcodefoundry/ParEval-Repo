# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ and uses OpenMP offloading for GPU execution.

## Prerequisites

C++ compiler with OpenMP support must be installed. A compatible NVIDIA GPU driver is also required.

## Build

To build microXOR, use `make`, setting `OMP_OFFLOAD_TARGET` as appropriate for your system. For example, the following will build microXOR for an NVIDIA GPU:
