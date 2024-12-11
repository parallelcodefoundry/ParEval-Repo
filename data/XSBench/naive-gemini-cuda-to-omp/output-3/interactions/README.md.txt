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

XSBench has been implemented with OpenMP offloading for use with offload-capable architectures (e.g., CPUs with accelerators).

## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command (assuming a suitable compiler and OpenMP offloading support are configured):

```bash
make
```

You can alter compiler settings in the included Makefile.  Ensure that your compiler supports OpenMP offloading and that necessary flags are set appropriately.

### Debugging, Optimization & Profiling

There are also a number of switches that can be set in the makefile. Here is a sample of the control panel at the top of the makefile:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
```
- Optimization enables optimization flags (adjust based on your compiler).
- Debugging enables debugging flags (adjust based on your compiler).
- Profiling enables profiling flags (adjust based on your profiler).  When profiling the code, you may
wish to significantly increase the number of lookups (with the -l
flag) in order to wash out the initialization phase of the code.


## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench
```
For non-default settings, XSBench supports the following command line options:  (Note that some options might require adjustments for OpenMP offloading.)

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
Sets the simulation method, either "history" or "event". These options represent the history based or event based algorithms respectively. The default is the history based method. These two methods represent different methods of parallelizing the Monte Carlo transport method.  The OpenMP implementation will utilize parallel regions to handle the parallelism inherent in each method.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model.  ... (rest of the description remains the same)

- **-G [grid type]**
... (rest of the description remains the same)

- **-k [kernel]**
The OpenMP version will use OpenMP directives to parallelize the kernels. The optimized kernel selection will likely affect the performance but the underlying parallelism is handled via OpenMP.


## Feature Discussion

### OpenMP Offloading Support

The OpenMP offloading implementation leverages OpenMP directives to parallelize and offload computations to an accelerator.  The Makefile and compilation process need to be adjusted to reflect this.  You may need to specify compiler flags for OpenMP offloading, device selection, etc.

### Verification Support
... (rest of the description remains the same)

### Binary File Support
... (rest of the description remains the same)


## Algorithms
... (rest of the section remains the same)


## Optimized Kernels
... (rest of the section remains the same)

## Citing XSBench
... (rest of the section remains the same)

## Development Team
... (rest of the section remains the same)
```
