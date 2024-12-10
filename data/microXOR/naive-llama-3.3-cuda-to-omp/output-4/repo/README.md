# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offload for heterogeneous execution on CPUs and accelerators (e.g., GPUs).

## Prerequisites

OpenMP and an accelerator card must be installed. For NVIDIA GPUs, the CUDA toolkit or compatible drivers are required. For other accelerators, check with your system administrator or vendor documentation.

## Building

To build microXOR using OpenMP Offload, you'll need a compiler that supports OpenMP 4.5 or later. You can use the following command: