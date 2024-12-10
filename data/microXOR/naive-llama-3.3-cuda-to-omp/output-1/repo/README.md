# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offload for parallel execution on accelerators.

## Prerequisites

A compiler supporting OpenMP 4.5 or later and an accelerator device (e.g., GPU) must be available.

## Build

To build microXOR, use a compiler that supports OpenMP offload, such as GCC or Intel Compiler. The exact compilation command may vary depending on the compiler and system configuration. For example: