# microXOR: XOR stencil micro-benchmark (OpenMP Offload)

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

## Prerequisites

- Your system should have OpenMP support.
- Your compiler should be able to offload code to the GPU using OpenMP and OpenACC.
- CUDA must be installed on your system for compiling and running the GPU version.

## Compilation Instructions

To compile this program, you can use a Makefile or directly compile with the following commands: