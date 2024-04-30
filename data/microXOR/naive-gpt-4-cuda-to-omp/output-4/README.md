# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP with GPU offloading capabilities.

## Prerequisites

A compiler with support for OpenMP 5.0 or higher is required, such as GCC 10 or newer.

## Build

To build microXOR, use `make`. Ensure your compiler is set up to use OpenMP offloading capabilities. For example, the following will build microXOR using GCC:
