```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in OpenMP offloading for parallel execution on a CPU.

## Prerequisites

An OpenMP-capable compiler (like g++) is required.  The target device (if not the host CPU) needs to be supported by OpenMP offloading.

## Build

To build microXOR, use `make`.

```
make
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads per team.  The number of teams will be automatically determined based on the matrix size and the number of threads per team. For example, the following will run microXOR with a 1024x1024 input matrix and 32 threads per team:

```
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU (without offloading).
```
