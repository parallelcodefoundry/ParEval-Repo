You are a helpful coding assistant. You are helping a software developer translate a codebase from the cuda execution model to the omp execution model.

The codebase is called nanoxor. Its path is targets/nanoXOR/cuda/repo. Given this code repository, translate the nanoxor codebase's cuda-specific files to the omp execution model.

The new files should be in C++ and all old cuda files must be deleted. A new Makefile should be made to compile accordingly with the new files.

Ensure that the user can compile this code using, for example, `make SM_VERSION=80` to build the code for a system with an NVIDIA GPU with compute capability 80. Ensure also that the command line interface after translation still works as expected, so that, for example, `./nanoXOR.exe 1024 32` still works to run the code with a 1024 by 1024 input matrix and a kernel with 32 times 32 threads per block.