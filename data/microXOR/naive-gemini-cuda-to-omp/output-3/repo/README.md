# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offloading for parallel execution on a CPU.

## Prerequisites

An OpenMP-capable compiler (like g++) is required.  The target device (if not the host CPU) needs to be supported by OpenMP offloading.

## Build

To build microXOR, use `make`.
