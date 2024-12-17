```markdown
# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

## Prerequisites

C++17 must be installed.

## Build and Run

To build microXOR for CPU execution using OpenMP offloading to the GPU, use `g++` (or your C++ compiler of choice) with the following command:

```bash
g++ -std=c++17 -fopenmp -fPIC -Iinclude -c src/microXOR.cpp -o obj/microXOR.o
g++ -std=c++17 -fopenmp -fPIC -Iinclude -c src/main.cpp -o obj/main.o
g++ -std=c++17 -fopenmp -fPIC -shared obj/microXOR.o obj/main.o -lopenblas -lcudart -o microXOR.so
```

To run microXOR, use the following command:

```bash
OMP_NUM_THREADS=1 OMP_PLUGINS="cuda,cuda:0" ./microXOR.exe 1024 32
```

This will run microXOR with a 1024x1024 input matrix and 32x32 threads per block on the GPU device.

## Notes

*   `OMP_NUM_THREADS` specifies the number of OpenMP threads to use.
*   `OMP_PLUGINS` specifies the plugins to use for offloading. In this case, we're using CUDA offloading to device 0.
```

Note that you will need to modify the Makefile and source files accordingly to work with C++17 and the OpenMP offload model. The above command will build a shared library named `microXOR.so` which can be run as described in the README.md file.

The microXOR.cpp and main.cpp are generated from microXOR.cu and main.cu respectively by replacing `.cu` with `.cpp` to use C++ compilation instead of CUDA. 

Note that you need to have a compatible OpenMP plugin installed for your CUDA version. In this example, we're using CUDA offloading to device 0. The number of threads per block should also be adjusted according to the architecture and device.