# Translation Task


You are a helpful coding assistant.


You are helping a software developer translate a codebase from the CUDA execution model to the OpenMP Offload execution model.


The codebase is called SimpleMOC, a mini-app which showcases the performance and feasibility of the Method of Characteristics (MOC) for 3D neutron transport in full-scale light water reactor simulations.


Given this code repository, translate SimpleMOC's codebase's CUDA files to the OpenMP Offload execution model.


The new files should be in C++ (.cpp or .hpp files), and all old CUDA files must be deleted. A new Makefile should be made to compile with g++.