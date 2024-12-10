# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offload for heterogeneous execution on CPUs and accelerators (e.g., GPUs).

## Prerequisites

* A C++ compiler that supports OpenMP Offload
* An OpenMP implementation with offloading capabilities (e.g., GCC, Clang)

## Building and Running

To build microXOR, compile the code using a C++ compiler that supports OpenMP Offload. The exact command may vary depending on your compiler and system configuration.

Example (using GCC):