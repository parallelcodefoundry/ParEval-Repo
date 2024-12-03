# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a CPU.

## Prerequisites

An OpenMP-capable compiler (like GCC or Clang) with OpenMP offloading support for your target device is required.  For example, you'll need to have appropriate libraries and configurations for offloading to an accelerator like an Intel Xeon Phi or similar.

## Build

To build microXOR, use `make`.  No specific architecture flags are needed as OpenMP will handle target selection based on your system configuration.
