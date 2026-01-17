# nanoXOR: XOR stencil nano-benchmark

This is nanoXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of nanoXOR is written in CUDA for GPU execution.

## Prerequisites

CUDA must be installed.

## Build

To build nanoXOR, use `make`, setting `CUDA_ARCH` as appropriate for your system. For example, the following will build nanoXOR for an NVIDIA GPU with compute capability 80.

```
make CUDA_ARCH=sm_80
```

## Run

nanoXOR requires two command-line arguments, one for matrix size and one for block size. For example, the following will run nanoXOR with a 1024x1024 input matrix and 32x32 threads per block:

```
./nanoXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
