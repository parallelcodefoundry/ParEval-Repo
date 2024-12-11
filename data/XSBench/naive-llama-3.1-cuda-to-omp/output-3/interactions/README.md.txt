Here is the translated README.md file for OpenMP-Offload:

```markdown
# XSBench: A Performance Abstraction for Monte Carlo Reactor Analysis

[![XSBench](docs/img/logo.png)](https://github.com/ANL-CESAR/XSBench)

[![Latest Github release](https://img.shields.io/github/release/ANL-CESAR/XSBench.svg)](https://github.com/ANL-CESAR/XSBench/releases/latest)
[![Build Status](https://travis-ci.com/ANL-CESAR/XSBench.svg?branch=master)](https://travis-ci.com/ANL-CESAR/XSBench)
[![Published in Annals of Nuclear Energy](https://img.shields.io/badge/Published%20in-Annals%20of%20Nuclear%20Energy-167DA4.svg)](https://www.sciencedirect.com/science/article/pii/S0306454914004332)

XSBench is a mini-app representing a key computational kernel of the Monte Carlo neutron transport algorithm. Specifically, XSBench represents the continuous energy macroscopic neutron cross section lookup kernel. XSBench serves as a lightweight stand-in for full neutron transport applications like [OpenMC](https://github.com/openmc-dev/openmc), and is a useful tool for performance analysis on high-performance computing architectures.

## Table of Contents

1. [Compilation](#Compilation)
2. [Running XSBench / Command Line Interface](#Running-XSBench)
3. [Feature Discussion](#Feature-Discussion)
	* [OpenMP Support](#OpenMP-Support)
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

## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command:

```bash
make
```
 
 You can alter compiler settings in the included Makefile.

### Debugging, Optimization & Profiling

There are also a number of switches that can be set in the makefile. Here is a sample of the control panel at the top of the makefile:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
```
- Optimization enables the -O3 optimization flag.
- Debugging enables the -g flag.
- Profiling enables the -pg flag. When profiling the code, you may wish to significantly increase the number of lookups (with the -l flag) in order to wash out the initialization phase of the code.

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
Sets the simulation method, either "history" or "event". These options represent the history based or event based algorithms respectively. The default is the history based method. These two methods represent different ways of parallelizing the Monte Carlo transport method. In the history based method, the central mode of parallelism is expressed over particles, which each require some number of macroscopic cross sections to be executed in series and in a dependent order. The event based method expresses its parallelism over a large pool of independent macroscopic cross section lookups that can be executed in any order without dependence.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model. There are four options: 'small', 'large', 'XL', and 'XXL'. By default, the 'large' option is selected. The H-M size corresponds to the number of nuclides present in the fuel region.

- **-g [gridpoints]**
Sets the number of gridpoints per nuclide. By default, this value is set to 11,303.

- **-G [grid type]**
Sets the grid search type (unionized, nuclide, hash). Defaults to unionized.

- **-p [particles]**
Sets the number of particle histories to simulate. By default, this value is set to 500,000.

- **-l [lookups]**
Sets the number of cross-section (XS) lookups to perform per particle. By default, this value is set to 34.

- **-h [hash bins]**
Sets the number of hash bins (only relevant when using the hash lookup algorithm).

- **-b [binary mode]**
This optional mode can read or write the simulation data structures to disk.

- **-k [kernel]**
There are several optimized variants of the main kernel. All source bases run basically the same "baseline" kernel as default. Optimized kernels can be selected at runtime with this argument.

## Feature Discussion

### OpenMP Support

XSBench has been implemented using the OpenMP framework for multi-threading and offloading. This allows users to parallelize the code on CPU architectures, as well as offload sections of the code to accelerators such as GPUs or FPGAs.

### Verification Support

Legacy versions of XSBench had a special "Verification" compiler flag option to enable verification of the results. However, a much more performant and portable verification scheme was developed and is now used for all configurations -- therefore, it is not necessary to compile with or without the verification mode as it is always enabled by default.

### Binary File Support

Instead of initializing the randomized synthetic cross section data structures in XSBench every time it is run, you may optionally have XSBench generate a data set and write it to file. It can then be read on subsequent runs to speed up initialization.

## Algorithms

### Transport Simulation Styles

#### History-Based Transport

The default simulation model used in XSBench is the "history-based" model. In this model, parallelism is expressed over independent particle histories, with each particle being simulated in a serial fashion from birth to death:

```c
for each particle do		   // Independent
	while particle is alive do // Dependent
		Move particle to collision site
		Process particle collision
```

This method of parallelism is very memory efficient, as the total number of particles that must be kept in memory at once is equivalent to the total number of active threads being run in the simulation.

#### Event-Based Transport

An alternative simulation model is the "event-based" model. In this model, parallelism is instead expressed over different collision (or "event") types. To facilitate this, all particles in the simulation are stored in memory at once. Each event kernel is executed in parallel on vectors of particles that currently require that event to be executed:

```c
Get vector of source particles
while any particles are alive do	     // Dependent
	for each living particle do          // Independent
		Move particle to collision site
	for each living particle do          // Independent
		Process particle collision
	Sort/consolidate surviving particles
```

This method of parallelism is requires more memory and requires an extra stream compaction kernel to sort and organize the particles periodically to ready them for the different event kernels. The benefit of this model is that kernels can potentially be execute in a SIMD manner and with higher cache efficiency due to the potential to sort particles by material and energy.

### Cross Section (XS) Lookup Methods

XSBench represents the macroscopic cross section lookup kernel. This kernel is responsible for adding together microscopic cross section data from all nuclides present in the material the neutron is travelling through, given a certain energy:

<p align="center"> <img src="docs/img/XS_equation.svg" alt="XS_Lookup_EQ" width="500"/> </p>

Macroscopic cross section data is typically required for multiple reaction channels "c", such as the total cross section, fission cross section, etc. This data is typically stored in point-wise data form for each nuclide.

#### Nuclide Grid

This is the default "naive" method of performing macroscopic XS lookups. XS data is stored for a number of energy levels for each nuclide in the simulation problem.

#### Unionized Energy Grid

One way of speeding up the nuclide grid search is to form a separate acceleration structure to reduce the number of binary searches that need to be performed.

#### Logarithmic Hash Grid

An alternative to the unionized energy grid is the logarithmic hash grid. This method takes in account the fact that while nuclides will be tabulated on grids containing different numbers of energy points, the points within each nuclide's grid will in general be spaced in (roughly) uniform maner in log space.

## Optimized Kernels

If using the event-based model, we will be executing the lookup kernel in XSBench across all particles at once. While SIMD execution is possible using this method, typically issues can arrise that greatly reduce SIMD efficiency.

## Citing XSBench

Papers citing the XSBench program in general should refer to:

>J. R. Tramm, A. R. Siegel, T. Islam, and M. Schulz, ���XSBench - The Development and Verification of a Performance Abstraction for Monte Carlo Reactor Analysis,��� presented at PHYSOR 2014 - The Role of Reactor Physics toward a Sustainable Future, Kyoto. https://www.mcs.anl.gov/papers/P5064-0114.pdf

Bibtex Entry:

```bibtex
@inproceedings{Tramm:wy,
author = {Tramm, John R and Siegel, Andrew R and Islam, Tanzima and Schulz, Martin},
title = {{XSBench} - The Development and Verification of a Performance Abstraction for {M}onte {C}arlo Reactor Analysis},
booktitle = {{PHYSOR} 2014 - The Role of Reactor Physics toward a Sustainable Future},
address = {Kyoto},
year = 2014,
url = "https://www.mcs.anl.gov/papers/P5064-0114.pdf"
}
```

## Development Team

Authored and maintained by John Tramm ([@jtramm](https://github.com/jtramm)) with help from Ron Rahaman, Amanda Lund, and other [contributors](https://github.com/ANL-CESAR/XSBench/graphs/contributors).
```