# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a supported accelerator (e.g., GPU).

## Prerequisites

An OpenMP-capable compiler with offloading support (e.g., GCC, Clang) and a compatible accelerator are required.  The specific requirements depend on your target accelerator.

## Build

To build microXOR, use `make`.  No special flags are needed for specifying the target architecture as OpenMP offloading handles target selection automatically (though you may need to set environment variables depending on your compiler and hardware).
