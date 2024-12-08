# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP for offloading execution to compatible devices.

## Prerequisites

OpenMP must be installed and your compiler must support OpenMP offloading.

## Build

To build microXOR, use `make`, setting `OMP_TARGET` as appropriate for your system. For example, the following will build microXOR for an OpenMP-compatible device.
