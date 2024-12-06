# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ for CPU execution and uses OpenMP offloading to utilize multi-core processors.

## Prerequisites

A compatible compiler that supports OpenMP must be installed. This example uses GCC with the `-fopenmp` flag.

## Compilation

To compile the code, use the following command: