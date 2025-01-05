# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ using the Kokkos parallel programming model.

## Prerequisites

Kokkos must be installed and configured.

## Build

To build microXOR, use your preferred build system (e.g., CMake), setting the necessary flags to enable Kokkos. For example, the following will build microXOR using CMake:
