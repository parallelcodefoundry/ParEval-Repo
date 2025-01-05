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
make
```

You can alter compiler settings in the included CMakeLists.txt.

### Debugging, Optimization & Profiling

Optimization and debugging flags can be controlled through CMake, for example:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release # For optimization
cmake .. -DCMAKE_BUILD_TYPE=Debug   # For debugging
```

Profiling can be done using tools like gprof (for CPU) or other architecture-specific profilers.


## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench
```
For non-default settings, XSBench supports the following command line options (many are the same as before, but the -k argument is likely removed or changed to select Kokkos execution space):

| Argument    |Description | Options     | Default
|-------------|------------|---------------|------------|
|-m           |Simulation method| history, event| history|
|-s | Problem Size | small, large, XL, XXL | large|
|-g | # of gridpoints per nuclide (overrides -s defaults) | integer value| 11,303 |
-G | Grid search type | unionized, nuclide, hash | unionized |
|-p | # of particle histories (if running using "history" method)| integer value | 500,000 |
|-l | # of Cross-section (XS) lookups. If using history based method, this is lookups per particle history. If using event-based method, this is total lookups. | integer value | (History: 34) (Event: 17,000,000) |
|-h | # of hash bins (only used with hash-based grid search) | integer value | 10,000 |
|-b | Read/Write binary files | read, write |  |
|  --csv <file path>        | Save output to csv file. (Default is stdout) | file path | stdout |
|-n <num iterations>      | Specifies how many kernel iterations to run. | integer value | 1 |
|-w <num warmups>         | Specifies how many warmup iterations to run. | integer value | 0 |


- **-m [simulation method]** (same as before)

- **-s [size]** (same as before)

- **-g [gridpoints]** (same as before)

- **-G [grid type]** (same as before)

- **-p [particles]** (same as before)

- **-l [lookups]** (same as before)

- **-h [hash bins]** (same as before)

- **-b [binary mode]** (same as before)


## Feature Discussion

### Kokkos Support

The CUDA code has been replaced with Kokkos kernels. This allows XSBench to run on various architectures supported by Kokkos, such as CPUs and multiple GPU vendors, without significant code changes.  The choice of Kokkos execution space (e.g., OpenMP, CUDA, etc.) will be determined during compilation, likely through CMake options.

### Verification Support (same as before)

### Binary File Support (same as before)


## Algorithms (same as before)


## Optimized Kernels

The optimized kernels from the CUDA version can be adapted to the Kokkos framework.  Kokkos's parallel for loops and other features can be used to express the parallelism more naturally.  The sorting operations (material and energy) can be implemented using Kokkos's parallel algorithms or external libraries that integrate with Kokkos.  The key is to leverage Kokkos to manage data movement and parallel execution efficiently across the chosen execution space.

## Citing XSBench (same as before)

## Development Team (same as before)
```
