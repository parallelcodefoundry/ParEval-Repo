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

Optimization and debugging flags can be controlled through CMake's build type settings (e.g., `Release`, `Debug`). Profiling can be done using tools like gprof or other profilers compatible with your chosen Kokkos backend.


## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench
```
For non-default settings, XSBench supports the following command line options (see `io.cpp` for details):

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
-n | Number of iterations | integer value | 1
-w | Number of warmup iterations | integer value | 0
--csv <file path> | Save output to CSV file | file path | stdout


- **-m [simulation method]**
Sets the simulation method, either "history" or "event".  These options represent the history-based or event-based algorithms respectively. The default is the history-based method. These two methods represent different methods of parallelizing the Monte Carlo transport method.  See the [Transport Simulation Styles](#Transport-Simulation-Styles) section for more information.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model. There are four options: 'small', 'large', 'XL', and 'XXL'. By default, the 'large' option is selected.  See the original README for details on the impact of this parameter.

- **-g [gridpoints]**
Sets the number of gridpoints per nuclide. This will override the number of default gridpoints set by the '-s' option.

- **-G [grid type]**
Sets the grid search type (unionized, nuclide, hash). Defaults to unionized. See the [Cross Section Lookup Methods](#Cross-Section-Lookup-Methods) section for more details.

- **-p [particles]**
Sets the number of particle histories to simulate.


- **-l [lookups]**
Sets the number of cross-section (XS) lookups to perform per particle (history-based) or total lookups (event-based).


- **-h [hash bins]**
Sets the number of hash bins (only relevant when using the hash lookup algorithm).


- **-b [binary mode]**
This optional mode can read or write the simulation data structures to disk.  See the original README for details.


- **-k [kernel]**
Select the Kokkos kernel implementation to use.


## Feature Discussion

### Kokkos Support

XSBench now utilizes Kokkos for parallel execution. Kokkos allows for portability across different hardware backends (CPUs, GPUs, etc.) by abstracting away hardware-specific details. The choice of backend is configured at compile time.

### Verification Support

XSBench generates a hash of the results for verification purposes.


### Binary File Support

The binary file support remains the same as in the original CUDA version.


## Algorithms

(The Algorithms section remains largely unchanged from the original README.)


## Optimized Kernels

(The Optimized Kernels section remains largely unchanged, but note that the optimizations will be implemented using Kokkos parallel constructs.)


## Citing XSBench

(The Citing XSBench section remains unchanged.)


## Development Team

(The Development Team section remains unchanged.)
```
