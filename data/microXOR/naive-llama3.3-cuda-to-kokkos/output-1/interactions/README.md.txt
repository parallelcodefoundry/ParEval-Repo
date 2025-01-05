```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ using the Kokkos parallel programming model.

## Prerequisites

Kokkos must be installed and configured properly.

## Build

To build microXOR, use your preferred build system (e.g., CMake), setting the necessary flags to enable Kokkos support. For example, if using CMake, you can create a `CMakeLists.txt` file with the following content:
```cmake
cmake_minimum_required(VERSION 3.10)
project(microXOR)

find_package(Kokkos REQUIRED)

add_executable(microXOR microXOR.cpp microXOR.hpp)
target_link_libraries(microXOR Kokkos::KokkosCore)
```
Then, you can build microXOR using:
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
## Run

microXOR requires two command-line arguments, one for matrix size and one for the number of threads to use. For example, the following will run microXOR with a 1024x1024 input matrix and 32 threads:
```bash
./microXOR 1024 32
```
You should see `Validation passed.` if the operation completed successfully. The output of the kernel is tested against the output of the same problem run on the CPU.
```