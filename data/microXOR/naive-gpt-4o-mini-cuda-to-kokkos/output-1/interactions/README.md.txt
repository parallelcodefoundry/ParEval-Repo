```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in Kokkos for parallel execution on various backends.

## Prerequisites

Kokkos must be installed and configured for your system. You can find installation instructions on the [Kokkos GitHub page](https://github.com/kokkos/kokkos).

## Build

To build microXOR, use `make`, setting the appropriate Kokkos backend. For example, the following will build microXOR for a system with Kokkos configured for OpenMP:

```
make KOKKOS_BACKEND=OpenMP
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for block size. For example, the following will run microXOR with a 1024x1024 input matrix and 32x32 threads per block:

```
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```