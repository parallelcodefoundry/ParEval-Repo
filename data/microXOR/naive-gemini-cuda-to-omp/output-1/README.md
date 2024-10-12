# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offload for execution on a GPU.

## Prerequisites

OpenMP 4.5 or later must be installed with support for offloading to a GPU.

## Build

To build microXOR, use `make`, setting `CUDA_ARCH` as appropriate for your system. For example, the following will build microXOR for an NVIDIA GPU with compute capability 80.
