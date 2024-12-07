```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a host with an accelerator device (e.g., GPU).

## Prerequisites

An OpenMP-capable compiler with offloading support (e.g., GCC, Clang) and an accelerator device are required.  The target architecture for offloading should be specified during compilation.

## Build

To build microXOR, use `make`.  The Makefile will attempt to automatically detect the appropriate offloading target, but you may need to adjust the `TARGET` variable if necessary. For example, to specify an NVIDIA GPU, you might need to use a compiler flag such as `-fopenmp-targets=nvptx64-nvidia-cuda`.

```bash
make
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads per block (this will affect the OpenMP parallel region, not CUDA blocks as before). For example, the following will run microXOR with a 1024x1024 input matrix and 32x32 threads per block (per OpenMP team):

```bash
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```
