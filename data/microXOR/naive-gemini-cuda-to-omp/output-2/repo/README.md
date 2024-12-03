# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a CPU.

## Prerequisites

An OpenMP-capable compiler is required (e.g., GCC, Clang with OpenMP support).  The target offload device should be specified appropriately.

## Build

To build microXOR, use `make`.  The Makefile can be adjusted to target a specific device, if needed (consult your compiler documentation for offloading device specifications).
