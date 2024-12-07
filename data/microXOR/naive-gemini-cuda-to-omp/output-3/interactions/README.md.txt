```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a CPU.

## Prerequisites

An OpenMP-capable compiler (like GCC or Clang) with OpenMP offloading support for your target device is required.

## Build

To build microXOR, use `make`.  No special flags are needed for specifying the target architecture as OpenMP handles target selection differently than CUDA.

```
make
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads per block (this translates to OpenMP's work-sharing constructs). For example, the following will run microXOR with a 1024x1024 input matrix and a suggested number of threads (experiment to find optimal):

```
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the offloaded kernel is tested against the output of the same problem run on the CPU.
```
