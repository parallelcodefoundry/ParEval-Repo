# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ with OpenMP directives for offloading to a device (GPU).

## Prerequisites

OpenMP and a compatible compiler must be installed. Additionally, CUDA or another device driver that supports OpenMP-offload must be installed on the target system.

## Building and Running

To build and run microXOR, use the following commands:
