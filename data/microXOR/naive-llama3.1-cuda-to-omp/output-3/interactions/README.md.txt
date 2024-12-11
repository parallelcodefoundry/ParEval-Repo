```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

## Prerequisites

The OpenMP compiler and a compatible offload target (e.g. GPU) must be installed.

## Build

To build microXOR, use the following command:

```bash
gcc -fopenmp main.c ooffload -lomp -ltarget
```

where `main.c` is the modified source file for the OpenMP offload version of the program, and `<target>` is the offload target (e.g. `-lcuda` for NVIDIA GPUs).

## Run

To run microXOR on an offload target, use the following command:

```bash
./microXOR <input> <output>
```

where `<input>` and `<output>` are the input and output files respectively.

Note: The source code has been modified to include OpenMP directives for offloading. See below for details.
```
```