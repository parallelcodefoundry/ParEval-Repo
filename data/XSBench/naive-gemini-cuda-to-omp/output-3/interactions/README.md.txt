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

XSBench has been implemented using OpenMP offload for compatibility with various architectures supporting OpenMP 5.0.


## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command (assuming a compiler supporting OpenMP 5.0 offloading is available):

```bash
make
```
 
 You can alter compiler settings in the included Makefile.  Ensure your compiler flags are appropriate for your target offload device.


### Debugging, Optimization & Profiling

The Makefile can be configured to enable debugging, optimization, and profiling (though profiling on the offload device might require specific tools for your hardware).  The relevant variables in the Makefile are:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
```
- Optimization enables compiler optimizations.
- Debugging enables debugging symbols.
- Profiling enables profiling information. When profiling the code, you may wish to significantly increase the number of lookups (with the -l flag) in order to wash out the initialization phase of the code.


## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench
```
For non-default settings, XSBench supports the following command line options:

| Argument    |Description | Options     | Default
|-------------|------------|---------------|------------|
|-m           |Simulation method| history, event| history|
|-s           |Problem Size   | small, large, XL, XXL | large|
|-g           |# of gridpoints per nuclide (overrides -s defaults) | integer value| 11,303 |
|-G           |Grid search type | unionized, nuclide, hash | unionized |
|-p           |# of particle histories (if running using "history" method)| integer value | 500,000 |
|-l           |# of Cross-section (XS) lookups. If using history based method, this is lookups per particle history. If using event-based method, this is total lookups. | integer value | (History: 34) (Event: 17,000,000) |
|-h           |# of hash bins (only used with hash-based grid search) | integer value | 10,000 |
|-b           |Read/Write binary files | read, write |  |
|-k           |Optimized kernel ID (OpenMP tasks) | integer value | 0
|-n           |Number of iterations       | integer value | 1

- **-m [simulation method]**  Sets the simulation method, either "history" or "event".  The default is the history-based method.

- **-s [size]** Sets the size of the Hoogenboom-Martin reactor model.  The options are 'small', 'large', 'XL', and 'XXL'.

- **-g [gridpoints]** Sets the number of gridpoints per nuclide.

- **-G [grid type]** Sets the grid search type (unionized, nuclide, hash).

- **-p [particles]** Sets the number of particle histories to simulate.

- **-l [lookups]** Sets the number of cross-section (XS) lookups to perform per particle (history-based) or total lookups (event-based).

- **-h [hash bins]** Sets the number of hash bins (only relevant when using the hash lookup algorithm).

- **-b [binary mode]** This optional mode can read or write the simulation data structures to disk.

- **-k [kernel]**  Specifies which kernel to run (different implementations using OpenMP tasks).  0 is baseline, other values are optimized variants.


## Feature Discussion

### OpenMP Offload Support

The OpenMP offload model will handle data movement and kernel execution on an accelerator device.  The `#pragma omp target` directive will be used to offload computationally intensive parts of the code.  The Makefile needs to be configured to compile with OpenMP support.

### Verification Support

XSBench generates a hash of the results, enabling verification across different runs and configurations.

### Binary File Support

The binary file support allows for faster initialization by reading or writing simulation data to a file.

## Algorithms

(The Algorithms section remains largely the same, as the underlying algorithms are not changed by the change in execution model.)

## Optimized Kernels

Optimized kernels will utilize OpenMP tasks for parallelism within the event-based model.  The sorting strategies for improving SIMD efficiency and cache re-use described in the CUDA version apply equally to the OpenMP offload version.

## Citing XSBench

(The Citing XSBench section remains the same.)

## Development Team

(The Development Team section remains the same.)
```
