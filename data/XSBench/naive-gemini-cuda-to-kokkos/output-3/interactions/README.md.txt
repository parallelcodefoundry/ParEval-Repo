```markdown
![XSBench](docs/img/logo.png)

[![Latest Github release](https://img.shields.io/github/release/ANL-CESAR/XSBench.svg)](https://github.com/ANL-CESAR/XSBench/releases/latest)
[![Build Status](https://travis-ci.com/ANL-CESAR/XSBench.svg?branch=master)](https://travis-ci.com/ANL-CESAR/XSBench)
[![Published in Annals of Nuclear Energy](https://img.shields.io/badge/Published%20in-Annals%20of%20Nuclear%20Energy-167DA4.svg)](https://www.sciencedirect.com/science/article/pii/S0306454914004332)

XSBench is a mini-app representing a key computational kernel of the Monte Carlo neutron transport algorithm. Specifically, XSBench represents the continuous energy macroscopic neutron cross section lookup kernel. XSBench serves as a lightweight stand-in for full neutron transport applications like [OpenMC](https://github.com/openmc-dev/openmc), and is a useful tool for performance analysis on high-performance computing architectures.

## Table of Contents

1. [Compilation](#Compilation)
2. [Running XSBench / Command Line Interface](#Running-XSBench)
3. [Feature Discussion](#Feature-Discussion)
	* [Kokkos Support](#Kokkos-Support)
	* [Verification Support](#Verification-Support)
	* [Binary File Support](#Binary-File-Support)
4. [Theory & Algorithms](#Algorithms)
	* [Transport Simulation Styles](#Transport-Simulation-Styles)
		- [History-Based Transport](#History-Based-Transport)
		- [Event-Based Transport](#Event-Based-Transport)
	* [Cross Section Lookup Methods](#Cross-Section-Lookup-Methods)
		- [Nuclide Grid](#Nuclide-Grid)
		- [Unionized Energy Grid](#Unionized-Energy-Grid)
		- [Logarithmic Hash Grid](#Logarithmic-Hash-Grid)
5. [Optimized Kernels](#Optimized-Kernels)
6. [Citing XSBench](#Citing-XSBench)
7. [Development Team](#Development-Team) 

XSBench has been implemented using Kokkos for execution on various architectures (CPUs, GPUs, etc.).


## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use a build system like CMake:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

You can alter compiler settings within the CMakeLists.txt file.  For example, you can specify Kokkos backends.

### Debugging, Optimization & Profiling

Optimization and debugging flags can be controlled through CMake's build types (e.g., `Release`, `Debug`) or directly in `CMakeLists.txt`.  Profiling tools appropriate for your chosen Kokkos backend should be used.

## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench
```
For non-default settings, XSBench supports the following command line options (passed to the executable):

| Argument    |Description | Options     | Default
|-------------|------------|---------------|------------|
|-m           |Simulation method| history, event| history|
|-s | Problem Size | small, large, XL, XXL | large|
|-g | # of gridpoints per nuclide (overrides -s defaults) | integer value| 11,303 |
-G | Grid search type | unionized, nuclide, hash | unionized |
-p | # of particle histories (if running using "history" method)| integer value | 500,000 |
-l | # of Cross-section (XS) lookups. If using history based method, this is lookups per particle history. If using event-based method, this is total lookups. | integer value | (History: 34) (Event: 17,000,000) |
-h | # of hash bins (only used with hash-based grid search) | integer value | 10,000 |
-b | Read/Write binary files | read, write |  |
-k | Optimized kernel ID | integer value | 0

The descriptions of the command line options remain the same as in the original CUDA version.


## Feature Discussion

### Kokkos Support

XSBench utilizes Kokkos for parallel execution, enabling portability across different architectures (CPUs, GPUs, etc.) without significant code changes.  The Kokkos backend is specified during compilation.


### Verification Support

XSBench's verification scheme remains unchanged from the CUDA version. A hash of the results is generated and displayed.  This ensures consistency across different implementations and platforms.

### Binary File Support

The binary file support for reading and writing simulation data also remains unchanged from the CUDA version.

## Algorithms

This section remains unchanged from the original CUDA version's README.md.

### Transport Simulation Styles

#### History-Based Transport

#### Event-Based Transport

### Cross Section (XS) Lookup Methods

#### Nuclide Grid

#### Unionized Energy Grid

#### Logarithmic Hash Grid

## Optimized Kernels

The optimization strategies discussed in the CUDA version's README.md will apply to the Kokkos version as well, though the implementation using Kokkos::parallel_for will be different.

## Citing XSBench

This section remains unchanged from the original CUDA version's README.md.

## Development Team

This section remains unchanged from the original CUDA version's README.md.
```
