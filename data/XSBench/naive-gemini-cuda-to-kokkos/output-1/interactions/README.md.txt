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

You can alter compiler settings in the included CMakeLists.txt file.

### Debugging, Optimization & Profiling

Optimization and debugging flags can be controlled through CMake's build options (e.g., `cmake .. -DCMAKE_BUILD_TYPE=Debug`).  Profiling can be done using tools compatible with your chosen Kokkos backend.


## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench
```
For non-default settings, XSBench supports the following command line options:  (See `io.cpp` for detailed argument parsing.)

| Argument    |Description | Options     | Default|
|-------------|------------|---------------|------------|
|-m           |Simulation method| history, event| history|
|-s           |Problem Size | small, large, XL, XXL | large|
|-g           |# of gridpoints per nuclide (overrides -s defaults) | integer value| 11,303 |
|-G           |Grid search type | unionized, nuclide, hash | unionized |
|-p           |# of particle histories (if running using "history" method)| integer value | 500,000 |
|-l           |# of Cross-section (XS) lookups. If using history based method, this is lookups per particle history. If using event-based method, this is total lookups. | integer value | (History: 34) (Event: 17,000,000) |
|-h           |# of hash bins (only used with hash-based grid search) | integer value | 10,000 |
|-b           |Read/Write binary files | read, write |  |
|-k           |Optimized kernel ID (Kokkos policy selection)| integer value | 0 |
|-n           |number of kernel iterations | integer value | 1 |
|-w           |number of warmup iterations | integer value | 0 |
|--csv        |Save output to csv file (Default is stdout)| file path | stdout |

- **-m [simulation method]**
Sets the simulation method, either "history" or "event".  These represent different parallelization strategies.  See [Transport Simulation Styles](#Transport-Simulation-Styles).

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model.  Options are 'small', 'large', 'XL', and 'XXL'.  See README for details on sizes.

- **-g [gridpoints]**
Sets the number of gridpoints per nuclide, overriding the default set by `-s`.

- **-G [grid type]**
Sets the grid search type (unionized, nuclide, hash).  See [Cross Section Lookup Methods](#Cross-Section-Lookup-Methods).

- **-p [particles]**
Sets the number of particle histories.

- **-l [lookups]**
Sets the number of cross-section lookups per particle (history-based) or total lookups (event-based).

- **-h [hash bins]**
Sets the number of hash bins for hash-based grid search.

- **-b [binary mode]**
Controls reading/writing of simulation data to/from a binary file. See README.

- **-k [kernel]**  
Kokkos policy selection for optimized kernels.  0 is the default.

## Feature Discussion

### Kokkos Support

XSBench uses Kokkos for parallel execution.  This allows for portability across different architectures (CPUs, GPUs, etc.) by selecting the appropriate Kokkos backend.  Kokkos' parallel_for and other parallel constructs replace CUDA kernels.

### Verification Support
XSBench generates a hash of the results for verification.  The hash should be consistent across different runs with the same parameters and Kokkos backends.

### Binary File Support

The binary file support allows for pre-generation of simulation data, similar to CUDA version.


## Algorithms

### Transport Simulation Styles

#### History-Based Transport

(Same as original README)

#### Event-Based Transport

(Same as original README)

### Cross Section Lookup Methods

#### Nuclide Grid

(Same as original README)

#### Unionized Energy Grid

(Same as original README)

#### Logarithmic Hash Grid

(Same as original README)

## Optimized Kernels

(Similar to original README, but mention Kokkos policies instead of CUDA kernels) Optimized kernels will leverage Kokkos' execution spaces and policies (e.g., `Kokkos::DefaultExecutionSpace`, `Kokkos::CudaExecutionSpace`) for optimized performance on different architectures.

## Citing XSBench

(Same as original README)

## Development Team
(Same as original README)
```
