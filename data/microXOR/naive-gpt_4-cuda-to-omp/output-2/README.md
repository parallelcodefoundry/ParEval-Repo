# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP with GPU Offload for execution on GPUs supporting OpenMP target offload.

## Prerequisites

A compiler with OpenMP 4.5 or higher support must be installed. Examples of such compilers include GCC (with support for offloading) and Clang.

## Build

To build microXOR, use `make`. Ensure that your compiler and environment are set up to support OpenMP GPU offloading. For example, the following will build microXOR using GCC:
