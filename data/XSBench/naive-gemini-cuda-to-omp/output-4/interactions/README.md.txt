```markdown
![XSBench](docs/img/logo.png)

[![Latest Github release](https://img.shields.io/github/release/ANL-CESAR/XSBench.svg)](https://github.com/ANL-CESAR/XSBench/releases/latest)
[![Build Status](https://travis-ci.com/ANL-CESAR/XSBench.svg?branch=master)](https://travis-ci.com/ANL-CESAR/XSBench)
[![Published in Annals of Nuclear Energy](https://img.shields.io/badge/Published%20in-Annals%20of%20Nuclear%20Energy-167DA4.svg)](https://www.sciencedirect.com/science/article/pii/S0306454914004332)

XSBench is a mini-app representing a key computational kernel of the Monte Carlo neutron transport algorithm. Specifically, XSBench represents the continuous energy macroscopic neutron cross section lookup kernel. XSBench serves as a lightweight stand-in for full neutron transport applications like [OpenMC](https://github.com/openmc-dev/openmc), and is a useful tool for performance analysis on high performance computing architectures.

## Table of Contents

1. [Compilation](#Compilation)
2. [Running XSBench / Command Line Interface](#Running-XSBench)
3. [Feature Discussion](#Feature-Discussion)
	* [OpenMP Offload Support](#OpenMP-Offload-Support)
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

XSBench has been implemented using OpenMP offloading for compatibility with various architectures supporting OpenMP 5.0.


## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command (assuming you have a suitable OpenMP compiler like GCC or Clang with OpenMP 5.0 support and a target device available):

```bash
make
```
 
 You can alter compiler settings in the included Makefile.  You'll likely need to specify compiler flags for OpenMP offloading to your target device (e.g., `-fopenmp-targets=your_device`).


### Debugging, Optimization & Profiling

There are also a number of switches that can be set in the makefile. Here is a sample of the control panel at the top of the makefile:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
```
- Optimization enables optimization flags (e.g., `-O3`).
- Debugging enables debugging flags (e.g., `-g`).
- Profiling enables profiling flags (e.g., `-pg`). When profiling the code, you may
wish to significantly increase the number of lookups (with the -l
flag) in order to wash out the initialization phase of the code.


## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench
```
For non-default settings, XSBench supports the following command line options:

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


- **-m [simulation method]**
Sets the simulation method, either "history" or "event". These options represent the history based or event based algorithms respectively. The default is the history based method. These two methods represent different methods of parallelizing the Monte Carlo transport method.  See the [Transport Simulation Styles](#Transport-Simulation-Styles) section for more information.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model.  See the previous CUDA version for details.

- **-g [gridpoints]**
Sets the number of gridpoints per nuclide. See the previous CUDA version for details.

- **-G [grid type]**
Sets the grid search type (unionized, nuclide, hash). See the previous CUDA version for details.

- **-p [particles]**
Sets the number of particle histories to simulate. See the previous CUDA version for details.

- **-l [lookups]**
Sets the number of cross-section (XS) lookups to perform per particle. See the previous CUDA version for details.

- **-h [hash bins]**
Sets the number of hash bins (only relevant when using the hash lookup algorithm, as selected with "-G hash"). See the previous CUDA version for details.

- **-b [binary mode]**
This optional mode can read or write the simulation data structures to disk. See the previous CUDA version for details.

- **-k [kernel]**
Optimized kernels (if implemented) can be selected at runtime with this argument.  The baseline kernel is the default (0).

## Feature Discussion

### OpenMP Offload Support

The OpenMP offloading model is used to perform the computationally intensive parts of the simulation on an accelerator. The Makefile and compilation process will need to be adjusted to target a specific device.  Data transfer between host and device memory is handled implicitly by OpenMP's offloading directives (e.g., `#pragma omp target`).


### Verification Support

See the previous CUDA version for details.

### Binary File Support

See the previous CUDA version for details.


## Algorithms

### Transport Simulation Styles

#### History-Based Transport

See the previous CUDA version for details.

#### Event-Based Transport

See the previous CUDA version for details.

### Cross Section (XS) Lookup Methods

#### Nuclide Grid

See the previous CUDA version for details.

#### Unionized Energy Grid

See the previous CUDA version for details.

#### Logarithmic Hash Grid

See the previous CUDA version for details.


## Optimized Kernels

The OpenMP implementation may utilize different optimization strategies compared to the CUDA version.  The focus will likely shift to effective data parallelization and minimizing data transfer overhead between host and device.


## Citing XSBench

See the previous CUDA version for details.

## Development Team

See the previous CUDA version for details.
```
