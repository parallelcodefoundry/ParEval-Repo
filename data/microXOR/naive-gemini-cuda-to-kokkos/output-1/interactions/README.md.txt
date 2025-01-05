```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using Kokkos for parallel execution on various architectures (CPU, GPU).

## Prerequisites

Kokkos must be installed.  You'll also need a C++ compiler with C++17 support.

## Build

To build microXOR, use `make`.  You can specify the Kokkos backend using the `KOKKOS_BACKEND` environment variable.  For example, the following will build microXOR for the CUDA backend:

```bash
KOKKOS_BACKEND=Cuda make
```

Other backends include `Serial`, `OpenMP`, `OpenMP`, and others depending on your Kokkos installation.  See the Kokkos documentation for details.

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads to use per team (analogous to CUDA's block size, but the exact mapping depends on the Kokkos backend). For example, the following will run microXOR with a 1024x1024 input matrix and teams of 32x32 threads:

```bash
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```
