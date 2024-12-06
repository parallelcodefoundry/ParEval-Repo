# microXOR: XOR stencil micro-benchmark (OpenMP/Offload)

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

## Prerequisites

A compiler with OpenMP support must be installed.

## Build

To build microXOR, use `make`, setting the `CUDA_ARCH` environment variable as appropriate for your system. The resulting executable will use the OpenMP/Offload feature to launch the kernel on a device (e.g., GPU) and execute the host code on the CPU.
