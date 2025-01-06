```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in Kokkos for GPU execution.

## Prerequisites

Kokkos must be installed, and a compatible compiler must be available.

## Build

To build microXOR, use `make`. The Makefile is set up to compile with Kokkos. For example, the following will build microXOR for a system with Kokkos installed:

```
make
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for block size. For example, the following will run microXOR with a 1024x1024 input matrix and 32x32 threads per block:

```
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```