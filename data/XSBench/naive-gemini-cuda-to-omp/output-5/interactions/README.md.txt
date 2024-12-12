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

XSBench has been implemented using OpenMP offload for compatibility with various architectures supporting OpenMP.

## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command (assuming a compiler supporting OpenMP offload is available):

```bash
make
```

You can alter compiler settings in the included Makefile.  You'll need to ensure your compiler and linker flags are correctly configured for OpenMP offload.

### Debugging, Optimization & Profiling

There are also a number of switches that can be set in the makefile. Here is a sample of the control panel at the top of the makefile:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
OPENMP_OFFLOAD = yes // Add this line
```
- Optimization enables optimization flags (e.g., `-O3`).
- Debugging enables debugging flags (e.g., `-g`).
- Profiling enables profiling flags (e.g., `-pg`). When profiling the code, you may wish to significantly increase the number of lookups (with the -l flag) in order to wash out the initialization phase of the code.
- OPENMP_OFFLOAD enables OpenMP offload support.


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
Sets the simulation method, either "history" or "event". These options represent the history-based or event-based algorithms respectively. The default is the history-based method. These two methods represent different methods of parallelizing the Monte Carlo transport method.  See the [Transport Simulation Styles](#Transport-Simulation-Styles) section for more information.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model. There are four options: 'small', 'large', 'XL', and 'XXL'. By default, the 'large' option is selected.  See the section below for more details.

- **-g [gridpoints]**
Sets the number of gridpoints per nuclide.  This option will override the number of default gridpoints as set by the '-s' option.

- **-G [grid type]**
Sets the grid search type (unionized, nuclide, hash). Defaults to unionized.  See the [Cross Section Lookup Methods](#Cross-Section-Lookup-Methods) section for more details.

- **-p [particles]**
Sets the number of particle histories to simulate.


- **-l [lookups]**
Sets the number of cross-section (XS) lookups to perform per particle.


- **-h [hash bins]**
Sets the number of hash bins (only relevant when using the hash lookup algorithm, as selected with "-G hash").


- **-b [binary mode]**
This optional mode can read or write the simulation data structures to disk. Options are ("read" or "write").


- **-k [kernel]**
There are several optimized variants of the main kernel.  The optimized variants are related to different ways of sorting the sampled values such that there is less thread divergence and much better cache re-usage when executing the lookup kernel on contiguous sorted elements. More details can be found in the [Optimized Kernels](#Optimized-Kernels) section.


## Feature Discussion

### OpenMP Offload Support

XSBench now uses OpenMP offload directives to target accelerators.  The Makefile will need to be adjusted to compile with a compiler supporting OpenMP offload (e.g., GCC, Clang, Intel compiler) and the appropriate flags for offloading.

### Verification Support

XSBench generates a hash of the results at the end of the simulation and displays it with the other data once the code has completed executing.

### Binary File Support

XSBench can generate and read a binary data file to speed up initialization.


## Algorithms

### Transport Simulation Styles

#### History-Based Transport

#### Event-Based Transport

### Cross Section (XS) Lookup Methods

#### Nuclide Grid

#### Unionized Energy Grid

#### Logarithmic Hash Grid


## Optimized Kernels

## Citing XSBench

## Development Team
```
