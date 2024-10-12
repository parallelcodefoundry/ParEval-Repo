# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offloading for GPU execution.

## Prerequisites

OpenMP 5.0 or later must be installed.

## Build

To build microXOR, use `make`, setting `CUDA_ARCH` as appropriate for your system. For example, the following will build microXOR for an NVIDIA GPU with compute capability 80.
