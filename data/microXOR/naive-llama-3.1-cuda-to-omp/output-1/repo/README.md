# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ for CPU and GPU execution using OpenMP-offload.

## Prerequisites

A compiler supporting OpenMP 4.5 or later (e.g., GCC 9.x or Clang 10.x) must be installed.
The offload-targets must also be installed on the system, which includes the device-specific headers and libraries for CPUs and GPUs.

## Building microXOR

To build microXOR, simply run:
