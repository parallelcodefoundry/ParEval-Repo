# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offload for execution on a device with OpenMP offload support (e.g., a GPU).

## Prerequisites

An OpenMP offload-capable device must be present.

## Build

To build microXOR, use `make`, setting `CUDA_ARCH` as appropriate for your system. For example, the following will build microXOR for an NVIDIA GPU with compute capability 80.
