```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offloading for GPU execution (if available).  It falls back to CPU execution if offloading is not supported.

## Prerequisites

An OpenMP-capable compiler with offloading support (e.g., g++ with -fopenmp) is required.  For GPU offloading, a compatible device (e.g., NVIDIA GPU with appropriate drivers) must be present.


## Build

To build microXOR, use `make`.

```bash
make
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads per team (analogous to CUDA's block size). For example, the following will run microXOR with a 1024x1024 input matrix and a team size of 32x32:

```bash
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```
