# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP with GPU Offload for execution on GPUs supporting OpenMP target offload.

## Prerequisites

A compiler with support for OpenMP 5.0 or higher is required, such as GCC (version 10 or higher) or LLVM/Clang.

## Build

To build microXOR, use `make`. Ensure your compiler is set up to use OpenMP target offload. For example, the following will build microXOR using GCC:
