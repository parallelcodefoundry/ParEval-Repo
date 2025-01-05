```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ for execution with the Kokkos parallel programming model.

## Prerequisites

A C++ compiler and the Kokkos library must be installed.

## Build

To build microXOR, use a build system such as CMake, setting the C++ compiler and Kokkos library as appropriate for your system. For example, the following will build microXOR with the GCC compiler and the Kokkos library:

```bash
cmake -DKokkos_ENABLE_OPENMP=ON -DCMAKE_CXX_COMPILER=g++ .
cmake --build .
```

## Run

microXOR requires two command-line arguments, one for matrix size and one for number of threads. For example, the following will run microXOR with a 1024x1024 input matrix and 32 threads:

```bash
./microXOR.exe 1024 32
```

You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.

Note: The number of threads can be controlled by setting the `OMP_NUM_THREADS` environment variable for OpenMP or the `KOKKOS_NUM_THREADS` environment variable for other Kokkos backends.
```