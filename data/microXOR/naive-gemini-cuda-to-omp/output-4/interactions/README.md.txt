```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written using OpenMP offloading for parallel execution on a supported accelerator (e.g., GPU).

## Prerequisites

An OpenMP-capable compiler with offloading support (e.g., GCC, Clang) and a compatible accelerator are required.  The specific requirements depend on your target accelerator.

## Build

To build microXOR, use `make`.  No special flags are needed for specifying the target architecture as OpenMP offloading handles target selection automatically (though you may need to set environment variables depending on your compiler and hardware).

```bash
make
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of OpenMP threads. For example, the following will run microXOR with a 1024x1024 input matrix and 64 OpenMP threads:

```bash
./microXOR.exe 1024 64
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```
