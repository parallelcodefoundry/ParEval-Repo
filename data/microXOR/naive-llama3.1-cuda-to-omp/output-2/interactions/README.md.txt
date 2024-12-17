```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ and uses OpenMP offloading for GPU execution.

## Prerequisites

C++ compiler with OpenMP support must be installed. A compatible NVIDIA GPU driver is also required.

## Build

To build microXOR, use `make`, setting `OMP_OFFLOAD_TARGET` as appropriate for your system. For example, the following will build microXOR for an NVIDIA GPU:

```bash
make OMP_OFFLOAD_TARGET=nvptx64-nvidia
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for block size. For example, the following will run microXOR with a 1024x1024 input matrix and 32x32 threads per block:

```bash
./microXOR 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.


In this version, we use OpenMP to parallelize the execution and the `omp offload` directive to target the GPU.
```