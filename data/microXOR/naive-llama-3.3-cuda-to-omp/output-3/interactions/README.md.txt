```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP-offload for heterogeneous execution on CPUs and accelerators (e.g., GPUs).

## Prerequisites

OpenMP and an offloading-capable compiler must be installed. The target device (e.g., GPU) must also be supported by the compiler and have the necessary drivers.

## Compiling and Running

To compile microXOR using OpenMP-offload, use a command similar to:
```bash
gcc -fopenmp -foffload=nvptx-none microXOR.c -o microXOR
```
Replace `nvptx-none` with the target device (e.g., `amdgcn-amdhsa`, `intelgfx`) as needed. The exact compilation command may vary depending on your system and compiler.

To run microXOR, simply execute the compiled binary:
```bash
./microXOR N blockEdge
```
Replace `N` with the size of the grid (must be a multiple of `blockEdge`) and `blockEdge` with the edge length of each block (between 2 and 32).

## Notes

* The OpenMP-offload version of microXOR uses the same validation logic as the original CUDA version.
* The performance characteristics may differ between the two versions, depending on the target device and system configuration.
```