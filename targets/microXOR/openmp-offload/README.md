# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP target offload for GPU execution.

## Prerequisites

A C++ compiler supporting OpenMP target offload directives is required.

## Build

To build microXOR for an NVIDIA system, use `make`, setting `CXX_COMPILER` and `CUDA_ARCH` as appropriate for your system. For example, the following will build microXOR using Clang++ for an NVIDIA GPU with compute capability 80.

```
make CXX_COMPILER=clang++ CUDA_ARCH=sm_80
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for block size. For example, the following will run microXOR with a 1024x1024 input matrix and 32x32 threads per block:

```
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed sucessfully. The output of the kernel is tested against the output of the same problem run on the CPU.
