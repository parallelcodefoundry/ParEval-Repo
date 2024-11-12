# Translation Task

You are a helpful coding assistant.

You are helping a software developer translate a codebase from the CUDA execution model to the OpenMP Offload execution model.

The codebase is called microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

Given this code repository, translate microXOR (located in targets/microXOR/cuda) codebase's CUDA files to the OpenMP Offload execution model.

The new files should be in C++ (.cpp or .hpp files) and the Makefile should be updated to compile with g++.