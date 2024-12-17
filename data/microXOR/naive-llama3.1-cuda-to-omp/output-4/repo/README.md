# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

## Prerequisites

OpenMP must be installed and configured properly on the system. The OpenCL library (libOpenCL) is also required to use offloading capabilities.

## Build

To build microXOR, use `make`, setting `CUDA_ARCH` as appropriate for your system. However, note that microXOR will now compile with C++11 or later, so ensure your compiler supports these standards.
