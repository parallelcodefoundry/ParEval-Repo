```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offload for heterogeneous execution on CPUs and accelerators (e.g., GPUs).

## Prerequisites

* A C++ compiler that supports OpenMP Offload
* An OpenMP implementation with offloading capabilities (e.g., GCC, Clang)

## Building and Running

To build microXOR, compile the code using a C++ compiler that supports OpenMP Offload. The exact command may vary depending on your compiler and system configuration.

Example (using GCC):
```bash
g++ -fopenmp -o microXOR microXOR.cpp
```
Run the executable with the desired input size (`N`) and block edge (`blockEdge`):
```bash
./microXOR N blockEdge
```
Replace `N` and `blockEdge` with the actual values you want to use.

## Notes

* The `blockEdge` parameter controls the tile size for parallelization. It should be a power of 2 for optimal performance.
* The `N` parameter must be divisible by `blockEdge`.
* For best results, ensure that your system has an OpenMP implementation with offloading capabilities installed and configured correctly.

## Example Use Case

To run microXOR with a 1024x1024 grid and a block edge of 16:
```bash
./microXOR 1024 16
```
This will execute the XOR stencil operation on the specified grid using OpenMP Offload, leveraging the available CPU and accelerator resources.
```