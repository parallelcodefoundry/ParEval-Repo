```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offloading for parallel execution on a CPU.

## Prerequisites

OpenMP must be installed and supported by your compiler.  You'll need a compiler that supports OpenMP offloading (e.g., GCC, Clang).

## Build

To build microXOR, use `make`.

```
make
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads per block (which will map to OpenMP threads).  For example, the following will run microXOR with a 1024x1024 input matrix and utilize 32 threads:

```
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the offloaded kernel is tested against the output of the same problem run on the CPU.
```
