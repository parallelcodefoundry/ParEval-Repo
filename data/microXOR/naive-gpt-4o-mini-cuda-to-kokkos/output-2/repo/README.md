# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in Kokkos for parallel execution on various hardware architectures.

## Prerequisites

Kokkos must be installed and configured for your system.

## Build

To build microXOR, use `make`, setting the appropriate compiler and flags for your system. For example, the following will build microXOR for a system with Kokkos support.
