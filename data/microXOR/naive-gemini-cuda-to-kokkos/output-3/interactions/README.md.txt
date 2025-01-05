```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using Kokkos for parallel execution on various architectures (CPU, GPU).

## Prerequisites

Kokkos must be installed.  You'll also need a C++ compiler supporting C++17 or later.

## Build

To build microXOR, use `make`.  The Makefile will automatically detect available Kokkos backends. You can specify a specific backend using the `KOKKOS_BACKEND` environment variable (e.g., `KOKKOS_BACKEND=OpenMP make`, `KOKKOS_BACKEND=CUDA make`).

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads per block (this parameter influences Kokkos's work distribution, though its exact effect depends on the backend). For example, the following will run microXOR with a 1024x1024 input matrix and a target of 32x32 threads per block:

```
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```
