```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a CPU.

## Prerequisites

A C++ compiler with OpenMP 5.0 support is required.  You'll also need a system capable of OpenMP offloading (e.g., a system with an accelerator like a manycore CPU).

## Build

To build microXOR, use `make`.

```bash
make
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of OpenMP threads. For example, the following will run microXOR with a 1024x1024 input matrix and 64 threads:


```bash
./microXOR.exe 1024 64
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.

```
