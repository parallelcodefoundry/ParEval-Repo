```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ for execution on a host with offloading to a target device using OpenMP.

## Prerequisites

A C++ compiler with OpenMP support and a compatible target device are required.

## Build

To build microXOR, use a C++ compiler with OpenMP support, such as GCC or Clang. For example, the following will build microXOR:

```bash
g++ -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda microXOR.cpp -o microXOR.exe
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for block size. For example, the following will run microXOR with a 1024x1024 input matrix and 32x32 threads per block:

```bash
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the host.

Note: The actual command to run the executable may vary depending on the system and the compiler used.
```