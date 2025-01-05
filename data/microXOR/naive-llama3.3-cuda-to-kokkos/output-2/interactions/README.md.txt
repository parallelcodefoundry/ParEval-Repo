```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ for execution with the Kokkos parallel programming model.

## Prerequisites

Kokkos must be installed.

## Build

To build microXOR, use your preferred build system (e.g., CMake), setting the necessary flags for your system. For example, the following CMake command will build microXOR with the Kokkos OpenMP backend:

```bash
cmake -DKokkos_ENABLE_OPENMP=ON ..
cmake --build .
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for number of threads. For example, the following will run microXOR with a 1024x1024 input matrix and 32 threads:

```bash
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.

Note: The number of threads can be controlled using the `KOKKOS_NUM_THREADS` environment variable. For example:

```bash
KOKKOS_NUM_THREADS=32 ./microXOR.exe 1024 32
```