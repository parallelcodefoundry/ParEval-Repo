```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offloading for parallel execution on a CPU.

## Prerequisites

A C++ compiler with OpenMP 5.0 support is required.  You may need to specify compiler flags to enable OpenMP offloading (e.g., `-fopenmp` for GCC or Clang).

## Build

To build microXOR, use `make`.

```bash
make
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads.  For example, the following will run microXOR with a 1024x1024 input matrix and utilizing all available threads:

```bash
./microXOR.exe 1024 
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.

```
