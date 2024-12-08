```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP Offloading for GPU execution.

## Prerequisites

A compiler with OpenMP Offloading support must be installed. This typically includes compilers like GCC or Clang with OpenMP support.

## Build

To build microXOR, use `make`, setting the appropriate compiler and flags for OpenMP Offloading. For example, the following will build microXOR using GCC:

```
make CC=gcc
```

Ensure that your compiler supports OpenMP Offloading and is configured correctly for your GPU.

## Run

microXOR requires two command-line arguments, one for matrix size and one for block size. For example, the following will run microXOR with a 1024x1024 input matrix and 32x32 threads per block:

```
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```
