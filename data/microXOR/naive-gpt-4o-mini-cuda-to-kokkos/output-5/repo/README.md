# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in Kokkos for GPU execution.

## Prerequisites

Kokkos must be installed and configured for your system.

## Build

To build microXOR, use `make`. The Makefile is set up to compile with Kokkos. For example, the following will build microXOR for a system with Kokkos support.
