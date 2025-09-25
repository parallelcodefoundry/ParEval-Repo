# microXORh: XOR stencil micro-benchmark

This is microXORh, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXORh is written in OpenMP target offload for GPU execution, with the kernel provided by a header file.

## Prerequisites

A C++ compiler supporting OpenMP target offload directives is required.

## Build

To build microXORh for an NVIDIA system, use `make`, setting `CXX_COMPILER` and `CUDA_ARCH` as appropriate for your system. For example, the following will build microXORh using Clang++ for an NVIDIA GPU with compute capability 80.

```
make CXX_COMPILER=clang++ CUDA_ARCH=sm_80
```

## Run

microXORh requires two command-line arguments, one for matrix size and one for block size. For example, the following will run microXORh with a 1024x1024 input matrix and 32x32 threads per block:

```
./microXORh.exe 1024 32
```

You should see `Validation passed.` if the operation completed sucessfully. The output of the kernel is tested against the output of the same problem run on the CPU.
