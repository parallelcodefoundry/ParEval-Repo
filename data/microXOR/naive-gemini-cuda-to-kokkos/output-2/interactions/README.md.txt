```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using Kokkos for parallel execution on various backends (e.g., OpenMP, CUDA, SYCL).

## Prerequisites

Kokkos must be installed.  You'll also need a suitable backend library depending on your target execution space (e.g., CUDA for GPU execution).

## Build

To build microXOR, use `make`, setting the Kokkos backend as appropriate for your system.  For example, the following will build microXOR for CUDA execution:

```bash
make KOKKOS_BACKEND=cuda
```

Other backends might be supported depending on your Kokkos installation (e.g., `KOKKOS_BACKEND=openmp` for OpenMP).


## Run

microXOR requires two command-line arguments, one for matrix size and one for the block size (which influences the Kokkos team size). For example, the following will run microXOR with a 1024x1024 input matrix and a team size that implicitly reflects a 32x32 block structure (the exact mapping depends on Kokkos's internal parallelization):

```bash
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```
