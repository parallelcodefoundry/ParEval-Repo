# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ for execution with the Kokkos parallel programming model.

## Prerequisites

A C++ compiler and the Kokkos library must be installed.

## Build

To build microXOR, use a build system such as CMake, setting the C++ compiler and Kokkos library as appropriate for your system. For example, the following will build microXOR with the GCC compiler and the Kokkos library:
