# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP Offload for GPU execution.

## Prerequisites

OpenMP must be supported by your compiler, and a compatible GPU must be available.

## Build

To build microXOR, use `make`, setting `OMP_TARGET` as appropriate for your system. For example, the following will build microXOR for an OpenMP target.
