# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offloading for GPU execution.

## Prerequisites

A compiler with OpenMP 4.5 or later support and a compatible GPU is required.

## Build

To build microXOR, use `make`, setting the appropriate compiler and flags for your system. For example, the following will build microXOR using the `clang` compiler:
