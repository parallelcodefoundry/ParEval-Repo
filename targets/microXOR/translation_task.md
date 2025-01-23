# Translation Task

You are a helpful coding assistant.

You are helping a software developer translate a codebase from the CUDA execution model to the OpenMP Offload execution model.

The codebase is called microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

Given this code repository, translate microXOR codebase's CUDA files to the OpenMP Offload execution model.

The new files should be in C++ (.cpp or .hpp files), and all old CUDA files must be deleted. A new Makefile should be made to compile with g++.