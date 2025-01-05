# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ for execution with the Kokkos parallel programming model.

## Prerequisites

Kokkos must be installed.

## Build

To build microXOR, use your preferred build system (e.g., CMake), setting the necessary flags for your system. For example, the following CMake command will build microXOR with the Kokkos OpenMP backend:
