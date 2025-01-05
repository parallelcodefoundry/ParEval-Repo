# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using Kokkos for parallel execution on various architectures (CPU, GPU).

## Prerequisites

Kokkos must be installed.  You'll also need a C++ compiler with C++17 support.

## Build

To build microXOR, use `make`.  You can specify the Kokkos backend using the `KOKKOS_BACKEND` environment variable.  For example, the following will build microXOR for the CUDA backend:
