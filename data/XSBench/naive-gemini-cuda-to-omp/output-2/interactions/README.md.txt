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

XSBench has been adapted for OpenMP offload to target various architectures supporting OpenMP 5.0.

## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command (adjust compiler and flags as needed for your system):

```bash
make
```

You can alter compiler settings in the included Makefile.  The Makefile will need to be updated to use the appropriate OpenMP compiler flags for offloading.


### Debugging, Optimization & Profiling

The Makefile can be modified to control debugging, optimization, and profiling.  For example:

```make
OPTIMIZE = yes  # Enables optimization flags (e.g., -O3)
DEBUG    = no   # Enables debugging flags (e.g., -g)
PROFILE  = no   # Enables profiling flags (e.g., -pg)
```

When profiling, consider increasing the number of lookups (-l flag) to minimize the impact of the initialization phase.


## Running XSBench

To run XSBench with default settings, use:

```bash
./XSBench
```

XSBench's command-line interface remains largely unchanged, with the exception of the removal of the CUDA-specific kernel selection (`-k`).  Optimized kernels may be handled differently depending on the OpenMP compiler's capabilities.  See the Makefile for specifics.

The command line options are largely unchanged:

| Argument    |Description | Options     | Default
|-------------|------------|---------------|------------|
|-m           |Simulation method| history, event| history|
|-s | Problem Size | small, large, XL, XXL | large|
|-g | # of gridpoints per nuclide (overrides -s defaults) | integer value| 11,303 |
|-G | Grid search type | unionized, nuclide, hash | unionized |
|-p | # of particle histories (if running using "history" method)| integer value | 500,000 |
|-l | # of Cross-section (XS) lookups. If using history based method, this is lookups per particle history. If using event-based method, this is total lookups. | integer value | (History: 34) (Event: 17,000,000) |
|-h | # of hash bins (only used with hash-based grid search) | integer value | 10,000 |
|-b | Read/Write binary files | read, write |  |
|-n <num iterations>      | Specifies how many kernel iterations to run. (1 is default.) | integer value | 1 |
|-w <num warmups>         | Specifies how many warmup iterations to run. (0 is default.) | integer value | 0 |
|--csv <file path>        | Save output to csv file. (Default is stdout) | file path | stdout |


- **-m [simulation method]**  (unchanged)
- **-s [size]** (unchanged)
- **-g [gridpoints]** (unchanged)
- **-G [grid type]** (unchanged)
- **-p [particles]** (unchanged)
- **-l [lookups]** (unchanged)
- **-h [hash bins]** (unchanged)
- **-b [binary mode]** (unchanged)


## Feature Discussion

### OpenMP Offload Support

The CUDA code has been replaced with OpenMP offload directives. The Makefile and compilation process will need to be adjusted accordingly to use an OpenMP-capable compiler and specify the target offload device (if necessary).  The choice of optimized kernel will likely be handled differently; the OpenMP compiler may perform auto-vectorization or other optimizations.


### Verification Support (unchanged)
### Binary File Support (unchanged)


## Algorithms (unchanged)


## Optimized Kernels

The optimized kernels from the CUDA version need to be adapted for OpenMP offload. The OpenMP compiler may provide auto-vectorization and other optimizations, potentially reducing the need for explicit kernel splitting and sorting strategies. The effectiveness of each optimization technique will need to be re-evaluated in the OpenMP offload context.


## Citing XSBench (unchanged)
## Development Team (unchanged)
```
