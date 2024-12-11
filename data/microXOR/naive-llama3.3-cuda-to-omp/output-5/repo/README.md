# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP for offloading computations to accelerators (e.g., GPUs).

## Prerequisites

A C/C++ compiler supporting OpenMP and an accelerator device are required.

## Build

To build microXOR, use your preferred C/C++ compiler with OpenMP support. For example: