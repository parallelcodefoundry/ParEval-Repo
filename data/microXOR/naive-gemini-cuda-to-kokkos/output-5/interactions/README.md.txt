```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using Kokkos for parallel execution on various backends (e.g., OpenMP, CUDA, OpenCL).

## Prerequisites

Kokkos must be installed.  You'll also need a suitable Kokkos backend (e.g., CUDA, OpenMP).

## Build

To build microXOR, use `make`, setting `KOKKOS_BACKEND` as appropriate for your system.  For example, the following will build microXOR for CUDA execution:

```
make KOKKOS_BACKEND=cuda
```

For OpenMP execution:

```
make KOKKOS_BACKEND=openmp
```


## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads per block (only relevant for CUDA/OpenCL backends). For example, the following will run microXOR with a 1024x1024 input matrix and 32x32 threads per block (if using a backend that supports this level of control):

```
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```
