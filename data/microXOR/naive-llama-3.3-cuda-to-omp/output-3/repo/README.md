# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP-offload for heterogeneous execution on CPUs and accelerators (e.g., GPUs).

## Prerequisites

OpenMP and an offloading-capable compiler must be installed. The target device (e.g., GPU) must also be supported by the compiler and have the necessary drivers.

## Compiling and Running

To compile microXOR using OpenMP-offload, use a command similar to: