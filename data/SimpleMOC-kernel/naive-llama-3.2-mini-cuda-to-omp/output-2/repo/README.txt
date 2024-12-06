README.txt

Introduction
------------

The Offload Model of OpenMP allows the programmer to specify regions of code that can be offloaded from the host CPU to a device (e.g., GPU, FPGA). This model uses a combination of OpenMP directives and CUDA C++ API calls.

Prerequisites
-------------

* CUDA 11.0 or later
* C++ compiler that supports OpenMP

Directives
------------

To offload regions of code to the device, use the `omp target` directive. The general syntax is: