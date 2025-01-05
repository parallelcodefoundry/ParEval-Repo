```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ for execution with the Kokkos parallel programming model.

## Prerequisites

Kokkos must be installed.

## Build

To build microXOR, use `make`, setting `KOKKOS_ARCH` as appropriate for your system. For example, the following will build microXOR for an NVIDIA GPU.

```bash
make KOKKOS_ARCH=NVCC
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for block size. For example, the following will run microXOR with a 1024x1024 input matrix and 32x32 threads per block:

```bash
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.

Note: The Kokkos version of microXOR uses a different parallelization strategy than the CUDA version. Instead of launching a kernel on a 2D grid of threads, Kokkos uses a parallel_for loop to execute the computation in parallel. The block size argument is used to control the number of threads used in the parallel_for loop.
```