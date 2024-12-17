# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

## Prerequisites

Your system must be equipped with offloading-capable hardware (e.g. Intel's GPU or NVIDIA's GPU) and the OpenMP development package installed.

## Build

To build microXOR, use `make`, setting `OPENMP_OFFLOAD` as appropriate for your system. For example, the following will build microXOR for an offloading-capable device:
