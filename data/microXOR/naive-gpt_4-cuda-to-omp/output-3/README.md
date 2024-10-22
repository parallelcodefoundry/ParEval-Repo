# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is adapted for execution using OpenMP with GPU offloading.

## Prerequisites

A compiler supporting OpenMP 4.5 or higher with GPU offload capabilities must be installed. Examples include recent versions of GCC (10 or higher) or LLVM/Clang.

## Build

To build microXOR, use `make`. Ensure your compiler is set up correctly in the Makefile to support OpenMP GPU offloading. For example, the following will build microXOR using GCC:
