# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP threads for CPU execution.

## Prerequisites

A C++ compiler supporting OpenMP directives is required.

## Build

To build microXOR, use `make`, setting `CXX_COMPILER` as appropriate for your compiler. For example, the following will build microXOR using Clang++:

```
make CXX_COMPILER=clang++
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for number of threads. For example, the following will run microXOR with a 1024x1024 input matrix and 16 threads:

```
./microXOR.exe 1024 16
```

You should see `Validation passed.` if the operation completed sucessfully. The output of the kernel is tested against the output of the same problem run on the CPU.
