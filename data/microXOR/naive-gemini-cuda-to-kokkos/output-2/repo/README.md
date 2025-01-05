# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using Kokkos for parallel execution on various backends (e.g., OpenMP, CUDA, SYCL).

## Prerequisites

Kokkos must be installed.  You'll also need a suitable backend library depending on your target execution space (e.g., CUDA for GPU execution).

## Build

To build microXOR, use `make`, setting the Kokkos backend as appropriate for your system.  For example, the following will build microXOR for CUDA execution:
