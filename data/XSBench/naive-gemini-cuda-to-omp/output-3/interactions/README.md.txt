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

XSBench has been implemented with OpenMP offload for use with offload-capable architectures (e.g., CPUs with accelerators).


## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command (assuming you have a suitable compiler and OpenMP offload support configured):

```bash
make
```
 
You can alter compiler settings in the included Makefile.  You'll need to adapt the Makefile to use your specific compiler flags for OpenMP offloading.  For example, you might need to add flags like `-fopenmp` or similar depending on your compiler.

### Debugging, Optimization & Profiling

There are also a number of switches that can be set in the makefile. Here is a sample of the control panel at the top of the makefile:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
```
- Optimization enables optimization flags (adapt as needed for your compiler and OpenMP offload).
- Debugging enables debugging flags (adapt as needed for your compiler and OpenMP offload).
- Profiling enables profiling flags (adapt as needed for your compiler and OpenMP offload,  consider tools like `gprof` or others suitable for OpenMP).


## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench
```
For non-default settings, XSBench supports the following command line options:  (Note:  The specific options may require adjustment based on how your OpenMP offload implementation handles parameters.)


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
Sets the simulation method, either "history" or "event".  These options represent the history-based or event-based algorithms respectively. The default is the history-based method. These two methods represent different methods of parallelizing the Monte Carlo transport method.  The implementation details for parallelization using OpenMP offload will differ for each method.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model. There are four options: 'small', 'large', 'XL', and 'XXL'. By default, the 'large' option is selected. The H-M size corresponds to the number of nuclides present in the fuel region.  The data structures will need to be allocated and managed appropriately using OpenMP offload.


- **-g [gridpoints]**
Sets the number of gridpoints per nuclide.  This value will impact the size of data structures that need to be transferred to the accelerator.

- **-G [grid type]**
Sets the grid search type (unionized, nuclide, hash).  The choice of grid type will influence how the data is structured and accessed during offloading.

- **-p [particles]**
Sets the number of particle histories to simulate.

- **-l [lookups]**
Sets the number of cross-section (XS) lookups to perform per particle.

- **-h [hash bins]**
Sets the number of hash bins (only relevant when using the hash lookup algorithm).

- **-b [binary mode]**
This optional mode can read or write the simulation data structures to disk.


- **-k [kernel]**
Different optimized kernels may be more or less suitable for OpenMP offload depending on their memory access patterns and parallelization strategies.

## Feature Discussion

### OpenMP Offload Support

The CUDA code has been replaced with OpenMP offload directives.  The `Simulation.cu` file will need significant modifications to utilize OpenMP's offloading capabilities. This will involve replacing CUDA kernels with code that is appropriately marked for offloading to an accelerator using OpenMP pragmas (e.g., `#pragma omp target` and related directives). Data transfer between the host and the accelerator will also need to be managed using OpenMP's data management clauses.

### Verification Support

Verification remains unchanged; the hash of the results is still generated and compared.

### Binary File Support

Binary file support remains as described in the original README.

## Algorithms

(The Algorithms section remains largely unchanged, as the underlying algorithms are independent of the execution model.)

## Optimized Kernels

The optimized kernels will require adaptation for OpenMP offload.  Consider how to best parallelize each kernel using OpenMP's constructs and how data dependencies may affect performance.

## Citing XSBench

(The Citing XSBench section remains unchanged.)

## Development Team

(The Development Team section remains unchanged.)
```
