# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a CPU.

## Prerequisites

An OpenMP-capable compiler (e.g., g++ with OpenMP support) is required.  The target device for offloading should be specified appropriately (e.g., using compiler flags like `-fopenmp-targets=host`).


## Build

To build microXOR, use `make`.  The Makefile will automatically handle compilation for the host and target device (if specified).


## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads to use (per OpenMP team). For example, the following will run microXOR with a 1024x1024 input matrix and 32 threads:
