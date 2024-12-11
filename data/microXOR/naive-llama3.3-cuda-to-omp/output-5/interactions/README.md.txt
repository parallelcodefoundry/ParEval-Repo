```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP for offloading computations to accelerators (e.g., GPUs).

## Prerequisites

A C/C++ compiler supporting OpenMP and an accelerator device are required.

## Build

To build microXOR, use your preferred C/C++ compiler with OpenMP support. For example:
```bash
gcc -fopenmp microXOR.c -o microXOR
```
Replace `microXOR.c` with the actual file name containing the source code.

## Run

Run the executable on a system with an accelerator device. The program will automatically detect and utilize available accelerators.
```bash
./microXOR N blockEdge
```
Replace `N` with the desired grid size and `blockEdge` with the preferred block edge length (between 2 and 32).

## Notes

* Ensure your compiler supports OpenMP offloading to accelerators.
* The program may require additional environment variables or configuration settings depending on the specific accelerator device and system architecture.
```