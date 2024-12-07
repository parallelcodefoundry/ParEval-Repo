# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a CPU.

## Prerequisites

A C++ compiler with OpenMP 5.0 support is required.  You'll also need a system capable of OpenMP offloading (e.g., a system with an accelerator like a manycore CPU).

## Build

To build microXOR, use `make`.
