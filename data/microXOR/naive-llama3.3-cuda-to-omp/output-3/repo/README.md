# microXOR: XOR stencil micro-benchmark

This is microXOR, a stencil computation benchmark computing an XOR operation over a 2D grid of cells.

This version of microXOR is written in C++ for execution on a host with offloading to a target device using OpenMP.

## Prerequisites

A C++ compiler supporting OpenMP 4.5 or later and a compatible target device are required.

## Build

To build microXOR, use a C++ compiler that supports OpenMP, such as GCC or Clang. For example, the following will build microXOR:
