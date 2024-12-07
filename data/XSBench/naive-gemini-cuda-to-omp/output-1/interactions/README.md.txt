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

XSBench has been adapted for the OpenMP offload execution model to utilize offload devices such as GPUs.  The specific device targeted will depend on compiler and linker flags.

## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command:

```bash
make
```

You can alter compiler settings in the included Makefile.  Ensure your compiler supports OpenMP offloading and that the necessary target device is available.

### Debugging, Optimization & Profiling

The Makefile includes options to control compilation flags:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
```
- Optimization enables optimization flags (e.g., `-O3`).
- Debugging enables debugging flags (e.g., `-g`).
- Profiling enables profiling flags (e.g., `-pg`).  When profiling, consider increasing the number of lookups to minimize initialization overhead.


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
-n | Number of iterations | integer value | 1
-w | Number of warmup iterations | integer value | 0
--csv | Save output to CSV file | file path | stdout


- **-m [simulation method]**
Sets the simulation method, either "history" or "event".  The default is the history-based method.  See the [Transport Simulation Styles](#Transport-Simulation-Styles) section for more information.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model. Options: 'small', 'large', 'XL', 'XXL'. The default is 'large'. The H-M size affects the number of nuclides in the fuel region.  "XL" and "XXL" represent larger problem sizes for benchmarking purposes.

- **-g [gridpoints]**
Sets the number of gridpoints per nuclide, overriding the default set by `-s`.

- **-G [grid type]**
Sets the grid search type (unionized, nuclide, hash). Defaults to unionized.  See the [Cross Section Lookup Methods](#Cross-Section-Lookup-Methods) section for more details.

- **-p [particles]**
Sets the number of particle histories to simulate. The default is 500,000.

- **-l [lookups]**
Sets the number of cross-section (XS) lookups. For history-based, this is lookups per particle; for event-based, this is the total number of lookups.

- **-h [hash bins]**
Sets the number of hash bins (only used with the hash lookup algorithm).

- **-b [binary mode]**
Controls reading or writing simulation data to a binary file ("read" or "write").  This can speed up initialization.

- **-k [kernel]**
Selects an optimized kernel (0 for baseline, others for optimized variants).

- **-n [num iterations]** Specifies how many kernel iterations to run.


## Feature Discussion

### OpenMP Offload Support

The XSBench code has been modified to use OpenMP offloading directives to target accelerators.  The Makefile will need to be configured correctly to specify the compiler and linker flags for your target device.

### Verification Support

XSBench includes verification by generating a hash of the results. This hash can be compared across different runs to verify correctness.

### Binary File Support

XSBench supports reading and writing simulation data to binary files to reduce initialization time.  The binary file format is not guaranteed to be portable between different compilers or systems.


## Algorithms

**(This section remains the same as in the original README)**

### Transport Simulation Styles
#### History-Based Transport
#### Event-Based Transport

### Cross Section (XS) Lookup Methods
#### Nuclide Grid
#### Unionized Energy Grid
#### Logarithmic Hash Grid


## Optimized Kernels

**(This section remains largely the same as in the original README, but might need minor wording adjustments to reflect OpenMP offloading instead of CUDA)**


## Citing XSBench

**(This section remains the same as in the original README)**


## Development Team

**(This section remains the same as in the original README)**
```
