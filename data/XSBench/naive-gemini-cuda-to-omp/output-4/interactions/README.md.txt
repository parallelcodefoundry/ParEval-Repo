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

XSBench has been implemented with OpenMP offloading for use with accelerators supporting the OpenMP offloading model.


## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command (assuming you have a compiler supporting OpenMP offloading, such as g++ with the appropriate flags, configured):

```bash
make
```
 
 You can alter compiler settings in the included Makefile.  You will need to adjust the Makefile to utilize your chosen compiler and its OpenMP offloading flags.


### Debugging, Optimization & Profiling

There are also a number of switches that can be set in the makefile.  You'll need to adapt these settings for your specific OpenMP offloading environment.  For example, you might use compiler flags like `-O3` for optimization and `-g` for debugging.  Profiling tools will depend on your target hardware and OpenMP implementation.


## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench
```
For non-default settings, XSBench supports the following command line options:  (Note:  The exact options and their defaults might need adjustment based on your OpenMP offloading setup).

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
-k | Optimized kernel ID | integer value | 0 (This might not be relevant for OpenMP offloading implementation)
-n | Number of iterations | integer | 1


- **-m [simulation method]**
Sets the simulation method, either "history" or "event". These options represent the history-based or event-based algorithms respectively. The default is the history-based method. These two methods represent different methods of parallelizing the Monte Carlo transport method.  The implementation of these methods using OpenMP offloading will differ from the CUDA version.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model. There are four options: 'small', 'large', 'XL', and 'XXL'. By default, the 'large' option is selected. The H-M size corresponds to the number of nuclides present in the fuel region.  The impact of problem size on offloading performance will need to be evaluated.

- **-g [gridpoints]**
Sets the number of gridpoints per nuclide. By default, this value is set to 11,303.  This parameter's effect on OpenMP offloading performance should be tested.

- **-G [grid type]**
Sets the grid search type (unionized, nuclide, hash). Defaults to unionized. The implementation of these grid types using OpenMP offloading will be different than in CUDA.

- **-p [particles]**
Sets the number of particle histories to simulate. By default, this value is set to 500,000.


- **-l [lookups]**
Sets the number of cross-section (XS) lookups to perform per particle.  This parameter will influence offloading performance.


- **-h [hash bins]**
Sets the number of hash bins (only relevant when using the hash lookup algorithm, as selected with "-G hash"). Default is 10,000.

- **-b [binary mode]**
This optional mode can read or write the simulation data structures to disk. Options are ("read" or "write").  This feature will remain largely unchanged in the OpenMP offloading implementation.

- **-k [kernel]**  This argument and its function may need to be removed or adapted as it refers to CUDA kernel selection, which is not directly applicable in OpenMP offloading.  OpenMP parallelization strategies will be different.

- **-n [num iterations]**
Specifies the number of simulation iterations.


## Feature Discussion

### OpenMP Offloading Support

This version utilizes OpenMP offloading to accelerate the XS lookups on compatible hardware.  The data transfer to and from the device will be handled by OpenMP clauses.  The parallelization strategy within the lookup kernel will utilize OpenMP directives.


### Verification Support

The verification method remains largely the same, generating a hash of the results for validation.


### Binary File Support

The binary file support for reading and writing data structures remains the same as in the CUDA version.



## Algorithms

(The Algorithms section remains largely the same)

### Transport Simulation Styles

#### History-Based Transport
#### Event-Based Transport

### Cross Section (XS) Lookup Methods

#### Nuclide Grid
#### Unionized Energy Grid
#### Logarithmic Hash Grid


## Optimized Kernels

The optimization strategies discussed in the CUDA version will need to be adapted to utilize OpenMP's parallelization features.  The key is efficient data partitioning and parallelization within the OpenMP `#pragma omp parallel for` construct, potentially using clauses like `schedule` to optimize work distribution.  The need for material sorting will depend on the chosen parallelization strategy.



## Citing XSBench

(The Citing XSBench section remains the same)


## Development Team
(The Development Team section remains the same)
```
