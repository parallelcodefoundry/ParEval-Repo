# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in Kokkos for parallel execution on various hardware architectures.

## Prerequisites

Kokkos must be installed. Ensure you have a compatible C++ compiler and the Kokkos library set up in your environment.

## Build

To build microXOR, use `make`, setting `KOKKOS_PATH` as appropriate for your system. For example, the following will build microXOR using Kokkos:
