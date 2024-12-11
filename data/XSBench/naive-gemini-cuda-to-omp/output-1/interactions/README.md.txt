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
	* [OpenMP Offloading Support](#OpenMP-Offloading-Support)
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

XSBench has been adapted for OpenMP offloading to target various architectures supporting OpenMP 5.0.

## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command (assuming a compiler supporting OpenMP 5.0 offloading):

```bash
make
```

You can alter compiler settings in the included Makefile.  Ensure your compiler and linker are configured appropriately for OpenMP offloading to your target device (e.g., setting environment variables).


### Debugging, Optimization & Profiling

The Makefile can control debugging, optimization, and profiling.  Adjust the following variables as needed:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
```
- `OPTIMIZE` enables optimization flags (adjust as needed for your compiler).
- `DEBUG` enables debugging flags (e.g., `-g`).
- `PROFILE` enables profiling flags (e.g., `-pg`; requires a suitable profiling tool).  Consider increasing lookups (-l) for better profiling results.

## Running XSBench

To run XSBench with default settings:
```bash
./XSBench
```

XSBench uses command-line options (see below).


## Running XSBench / Command Line Interface

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
Sets the simulation method, either "history" or "event".  The default is the history-based method.  See the [Transport Simulation Styles](#Transport-Simulation-Styles) section for more information.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model ('small', 'large', 'XL', 'XXL').  The default is 'large'.  See the description in the original README for details on problem size implications.

- **-g [gridpoints]**
Sets the number of gridpoints per nuclide.  Overrides '-s' defaults.

- **-G [grid type]**
Sets the grid search type ('unionized', 'nuclide', 'hash').  The default is 'unionized'.  See the [Cross Section Lookup Methods](#Cross-Section-Lookup-Methods) section for more details.

- **-p [particles]**
Sets the number of particle histories (only for "history" method).

- **-l [lookups]**
Sets the number of cross-section (XS) lookups per particle (history-based) or total lookups (event-based).

- **-h [hash bins]**
Sets the number of hash bins (only for "hash" grid type).

- **-b [binary mode]**
Read or write simulation data to/from a binary file ('read', 'write'). See original README for details.

- **-k [kernel]**
Selects which kernel to run (0 for baseline, 1, 2, etc. for optimized variants).


## Feature Discussion

### OpenMP Offloading Support

The CUDA code has been replaced with OpenMP offloading directives.  The Makefile should be adjusted to use a compiler supporting OpenMP 5.0 and the appropriate offloading targets.  You will likely need to specify compiler flags and environment variables to point to the correct OpenMP offloading libraries and target devices.


### Verification Support

XSBench uses a verification scheme that generates a hash of the results.  This hash can be compared to verify the correctness of the results across different runs and configurations (see original README for details).

### Binary File Support

The binary file support remains the same as described in the original README.

## Algorithms

### Transport Simulation Styles

#### History-Based Transport

(Same as in original README)

#### Event-Based Transport

(Same as in original README)

### Cross Section (XS) Lookup Methods

#### Nuclide Grid

(Same as in original README)

#### Unionized Energy Grid

(Same as in original README)

#### Logarithmic Hash Grid

(Same as in original README)

## Optimized Kernels

(Same as in original README)

## Citing XSBench

(Same as in original README)

## Development Team

(Same as in original README)
```
