Here is the translated README.md file for OpenMP-Offload execution model:

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

XSBench has been implemented in OpenMP for use with offloading to accelerators like GPUs and TPUs. NOTE: You will likely want to specify the accelerator device when compiling.

## Compilation

To compile XSBench with default settings, navigate to your selected source directory and use the following command:

```bash
make openmp
```
 
 You can alter compiler settings in the included Makefile. 

### Debugging, Optimization & Profiling

There are also a number of switches that can be set in the makefile. Here is a sample of the control panel at the top of the makefile:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
OPENMP   = yes
```
- Optimization enables the -O3 optimization flag.
- Debugging enables the -g flag.
- Profiling enables the -pg flag. When profiling the code, you may wish to significantly increase the number of lookups (with the -l flag) in order to wash out the initialization phase of the code.
- OpenMP enables OpenMP support in the code.

## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench openmp
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
Sets the simulation method, either "history" or "event". These options represent the history based or event based algorithms respectively. The default is the history based method. These two methods represent different methods of parallelizing the Monte Carlo transport method. In the history based method, the central mode of parallelism is expressed over particles, which each require some number of macroscopic cross sections to be executed in series and in a dependent order. The event based method expresses its parallelism over a large pool of independent macroscopic cross section lookups that can be executed in any order without dependence. They key difference between the two methods is the dependence/independence of the macroscopic cross section loop. See the [Transport Simulation Styles](#Transport-Simulation-Styles) section for more information.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model. There are four options: 'small', 'large', 'XL', and 'XXL'. By default, the 'large' option is selected. The H-M size corresponds to the number of nuclides present in the fuel region. The small version has 34 fuel nuclides, whereas the large version has 321 fuel nuclides. This significantly slows down the runtime of the program as the data structures are much larger, and more lookups are required whenever a lookup occurs in a fuel material. Note that the program defaults to "Large" if no specification is made. The additional size options, "XL" and "XXL", do not directly correspond to any particular physical model. They are similar to the H-M "large" option, except the number of gridpoints per nuclide has been increased greatly. This creates an extremely large energy grid data structure (XL: 120GB, XXL: 252GB), which is unlikely to fit on a single node, but is useful for experimentation purposes on novel architectures.

- **-g [gridpoints]**
Sets the number of gridpoints per nuclide. By default, this value is set to 11,303. This corresponds to the average number of actual gridpoints per nuclide in the H-M Large model as run by OpenMC with the actual ACE ENDF cross-section data. Note that this option will override the number of default gridpoints as set by the '-s' option.

- **-G [grid type]**
Sets the grid search type (unionized, nuclide, hash). Defaults to unionized. The unionized grid is what is typically used in Monte Carlo codes, as it offers the fastest speed. However, the increase in speed comes in a significant increase in memory usage as a union of all the separate nuclide grids must be formed and stored in memory. The "nuclide" mode uses only the basic nuclide grid data, with no unionization. This is slower as a binary search must be performed on every nuclide for each macroscopic XS lookup, rather than only once when using the unionized grid. Finally, the "hash" mode is a newer algorithm now used by many full Monte Carlo codes which offers speed nearly equivalent to the unionized energy grid method, but with only a small fraction of the memory overhead. See the [Cross Section Lookup Methods](#Cross-Section-Lookup-Methods) section for more details.

- **-p [particles]**
Sets the number of particle histories to simulate. By default, this value is set to 500,000. Users may want to increase this value if they wish to extend the runtime of XSBench, perhaps to produce more reliable performance counter data - as extending the run will decrease the percentage of runtime spent on initialization. Real MC simulations in a full application may use up to several billion particles per generation, so there is great flexibility in this variable.

- **-l [lookups]**
Sets the number of cross-section (XS) lookups to perform per particle. By default, this value is set to 34, which represents the average number of XS lookups per particle over the course of its lifetime in a light water reactor problem. Users should only alter this value if they are trying to capture the behavior of a different type of reactor (e.g., one with a fast spectrum), where the number of lookups per history may be different.

- **-h [hash bins]**
Sets the number of hash bins (only relevant when using the hash lookup algorithm, as selected with "-G hash"). Default is 10,000.

- **-b [binary mode]**
This optional mode can read or write the simulation data structures to disk. Options are ("read" or "write"). This may be useful if it is necessary to minimize the initialization phase of the program, which has a non-trivial runtime. The generated file is named "XS_data.dat" and will be located in the current working directory. The same file name and location will be used when reading. Note that as the file is binary, it may not be portable between compilers and computer systems. NOTE: When running in the "read" mode, you must be running with an identical program configuration as when the file was generated. E.g., if the file was generated with the "-G nuclide" argument, subsequent runs reading from that file must use the same configuration flags.

- **-k [kernel]**
There are several optimized variants of the main kernel. All source bases run basically the same "baseline" kernel as default. Optimized kernels can be selected at runtime with this argument. Default is "0" for the baseline, other variants are numbered 1, 2, ... etc. People interested in implementing their own optimized variants are encouraged to use this interface for convenience rather than writing over the main kernel. The baseline kernel is defined at the top of the "Simulation.c" source file, with the other variants being defined towards the end of the file after a large comment block delineation. The optimized variants are related to different ways of sorting the sampled values such that there is less thread divergence and much better cache re-usage when executing the lookup kernel on contiguous sorted elements. More details can be found in the [Optimized Kernels](#Optimized-Kernels) section.

## Feature Discussion

### OpenMP Support

XSBench now supports parallelization via OpenMP, which allows users to offload tasks to accelerators like GPUs and TPUs. This is a "weak scaling" approach -- for instance, if running the event-based model all threads will execute 17,000,000 cross section lookups regardless of how many threads are used. There is only one point of OpenMP communication (a reduce) at the end, which aggregates the timing statistics and averages them across threads before printing them out.

### Verification Support

Legacy versions of XSBench had a special "Verification" compiler flag option to enable verification of the results. However, a much more performant and portable verification scheme was developed and is now used for all configurations -- therefore, it is not necessary to compile with or without the verification mode as it is always enabled by default. XSBench generates a hash of the results at the end of the simulation and displays it with the other data once the code has completed executing. This hash can then be verified against hashes that other versions or configurations of the code generate.

### Binary File Support

Instead of initializing the randomized synthetic cross section data structures in XSBench every time it is run, you may optionally have XSBench generate a data set and write it to file. It can then be read on subsequent runs to speed up initialization. This process is controlled with the "-b (read, write)" command line argument.

## Theory & Algorithms

### Transport Simulation Styles

#### History-Based Transport

The default simulation model used in XSBench is the "history-based" model. In this model, parallelism is expressed over independent particle histories, with each particle being simulated in a serial fashion from birth to death: 

```c
for each particle do		   // Independent
	while particle is alive do // Dependent
		Move particle to collision site
		Process particle collision
```

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

### Cross Section (XS) Lookup Methods

XSBench represents the macroscopic cross section lookup kernel. This kernel is responsible for adding together microscopic cross section data from all nuclides present in the material the neutron is travelling through, given a certain energy.

#### Nuclide Grid

This is the default "naive" method of performing macroscopic XS lookups. XS data is stored for a number of energy levels for each nuclide in the simulation problem. Different nuclides can have a different number of energy levels. For instance, U-238 usually has over 100k energy levels, whereas some other nuclides may only have a few thousand.

#### Unionized Energy Grid

One way of speeding up the nuclide grid search is to form a separate acceleration structure to reduce the number of binary searches that need to be performed. In the Unionized Energy Grid (EUG) method, a second grid is created with columns corresponding to the union of all energy levels from the nuclide grid.

#### Logarithmic Hash Grid

An alternative to the unionized energy grid is the logarithmic hash grid. This method takes in account the fact that while nuclides will be tabulated on grids containing different numbers of energy points, the points within each nuclide's grid will in general be spaced in (roughly) uniform maner in log space .

## Optimized Kernels

If using the event-based model, we will be executing the lookup kernel in XSBench across all particles at once. While SIMD execution is possible using this method, typically issues can arrise that greatly reduce SIMD efficiency.

## Citing XSBench

Papers citing the XSBench program in general should refer to:

>J. R. Tramm, A. R. Siegel, T. Islam, and M. Schulz, ���XSBench - The Development and Verification of a Performance Abstraction for Monte Carlo Reactor Analysis,��� presented at PHYSOR 2014 - The Role of Reactor Physics toward a Sustainable Future, Kyoto.

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

Note that the rest of the codebase (Makefile, GridInit.cu, XSutils.cu, Materials.cu, etc.) will also need to be translated from CUDA to OpenMP-Offload execution model. This is a significant task and may require modifications to the existing code. 

Here's an example of how the `GridInit.cu` file might look like in OpenMP-Offload:
```c
#include "XSbench_header.h"

// ...

#pragma omp declare target

void grid_init_do_not_profile( Inputs in, int mype ) {
    // ...
}

void move_simulation_data_to_device( Inputs in, int mype, SimulationData SD ) {
    // ...
}

#pragma omp end target

int main() {
    // ...
}
```
Note the `#pragma omp declare target` and `#pragma omp end target` directives which are used to specify the offload regions of code. The `grid_init_do_not_profile` and `move_simulation_data_to_device` functions are marked as offloaded using these directives.

Similarly, the other files in the codebase will need to be modified to use OpenMP-Offload execution model. This may involve adding or modifying pragmas, function declarations, and variable definitions to support offloading.

It's worth noting that translating a CUDA program to an OpenMP-Offload program can be complex and time-consuming, especially if the original code is large and has many interdependent components. It's recommended to start with small, self-contained pieces of code and gradually build up the translation process.