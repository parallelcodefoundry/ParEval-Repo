# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in Kokkos for parallel execution on various backends.

## Prerequisites

Kokkos must be installed and configured for your system. You can find installation instructions on the [Kokkos GitHub page](https://github.com/kokkos/kokkos).

## Build

To build microXOR, use `make`, setting the appropriate Kokkos backend. For example, the following will build microXOR for a system with Kokkos configured for OpenMP:
