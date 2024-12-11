```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offload for parallel execution on accelerators.

## Prerequisites

A compiler supporting OpenMP 4.5 or later and an accelerator device (e.g., GPU) must be available.

## Build

To build microXOR, use a compiler that supports OpenMP offload, such as GCC or Intel Compiler. The exact compilation command may vary depending on the compiler and system configuration. For example:
```bash
gcc -fopenmp -o microXOR microXOR.c -foffload=nvptx-none
```
Replace `microXOR.c` with the actual source file name.

## Run

To run microXOR, ensure that the accelerator device is properly installed and configured. The program can be executed as follows:
```bash
./microXOR N blockEdge
```
Replace `N` with the size of the grid and `blockEdge` with the edge length of each block.

Note: The `N` must be divisible by `blockEdge`, and `blockEdge` should be between 2 and 32. Additionally, `N` must be at least 4.
```