# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offloading for GPU execution.

## Prerequisites

A compiler with OpenMP 4.5 or later support and a compatible GPU is required. Examples include GCC 9+ or Clang 9+.

## Build

To build microXOR, use `make`. Ensure that your compiler supports OpenMP Offloading and is configured correctly for your GPU. You may need to set environment variables or modify the Makefile to specify the target architecture.
