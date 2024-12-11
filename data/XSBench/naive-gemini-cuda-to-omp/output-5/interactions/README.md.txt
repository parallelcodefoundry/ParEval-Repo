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

To compile XSBench with default settings, navigate to your selected source directory and use the following command (assuming you have a compiler supporting OpenMP offloading, like g++ with the appropriate flags):

```bash
make
```
 
 You can alter compiler settings in the included Makefile.  You will likely need to adjust compiler flags to specify the target architecture for offloading.


### Debugging, Optimization & Profiling

There are also a number of switches that can be set in the makefile. Here is a sample of the control panel at the top of the makefile:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
```
- Optimization enables optimization flags (specific flags will depend on your compiler).
- Debugging enables debugging flags (specific flags will depend on your compiler).
- Profiling enables profiling flags (specific flags will depend on your profiler and compiler). When profiling the code, you may
wish to significantly increase the number of lookups (with the -l
flag) in order to wash out the initialization phase of the code.

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
Sets the simulation method, either "history" or "event". These options represent the history-based or event-based algorithms respectively. The default is the history-based method. These two methods represent different methods of parallelizing the Monte Carlo transport method. In the history-based method, the central mode of parallelism is expressed over particles, which each require some number of macroscopic cross sections to be executed in series and in a dependent order. The event-based method expresses its parallelism over a large pool of independent macroscopic cross section lookups that can be executed in any order without dependence. They key difference between the two methods is the dependence/independence of the macroscopic cross section loop. See the [Transport Simulation Styles](#Transport-Simulation-Styles) section for more information.

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

### OpenMP Offloading Support

XSBench now supports OpenMP offloading.  The computationally intensive parts of the code are offloaded to an accelerator device.  The Makefile and compilation process will need to be adjusted to correctly utilize the OpenMP offloading features of your compiler.

### Verification Support

(This section remains the same as in the original README.md)

### Binary File Support

(This section remains the same as in the original README.md)


## Algorithms

(This section remains the same as in the original README.md)

## Optimized Kernels

(This section remains the same as in the original README.md)

## Citing XSBench

(This section remains the same as in the original README.md)

## Development Team
(This section remains the same as in the original README.md)
```
