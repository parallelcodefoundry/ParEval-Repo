# microXORh: XOR stencil micro-benchmark

This is microXORh, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXORh is written in OpenMP threads for CPU execution.

## Prerequisites

A C++ compiler supporting OpenMP directives is required.

## Build

To build microXORh, use `make`, setting `CXX_COMPILER` as appropriate for your compiler. For example, the following will build microXORh using Clang++:

```
make CXX_COMPILER=clang++
```

## Run

microXORh requires two command-line arguments, one for matrix size and one for number of threads. For example, the following will run microXORh with a 1024x1024 input matrix and 16 threads:

```
./microXORh.exe 1024 16
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
