Here is the translation of the README.md file to the OpenMP-offload execution model:

```markdown
===============================
OpenMP-Offload XSBench
===============================

[![XSBench](docs/img/logo.png)](https://github.com/ANL-CESAR/XSBench)

[![Latest Github release](https://img.shields.io/github/release/ANL-CESAR/XSBench.svg)](https://github.com/ANL-CESAR/XSBench/releases/latest)
[![Build Status](https://travis-ci.com/ANL-CESAR/XSBench.svg?branch=master)](https://travis-ci.com/ANL-CESAR/XSBench)
[![Published in Annals of Nuclear Energy](https://img.shields.io/badge/Published%20in-Annals%20of%20Nuclear%20Energy-167DA4.svg)](https://www.sciencedirect.com/science/article/pii/S0306454914004332)

XSBench is a mini-app representing a key computational kernel of the Monte Carlo neutron transport algorithm. Specifically, XSBench represents the continuous energy macroscopic neutron cross section lookup kernel. XSBench serves as a lightweight stand-in for full neutron transport applications like [OpenMC](https://github.com/openmc-dev/openmc), and is a useful tool for performance analysis on high performance computing architectures.

## Table of Contents

1. [Compilation](#Compilation)
2. [Running XSBench / Command Line Interface](#Running-XSBench)
3. [Feature Discussion](#Feature-Discussion)
	* [MPI Support](#MPI-Support)
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
make openmp-offload
```

You can alter compiler settings in the included `Makefile`.

### Debugging, Optimization & Profiling

There are also a number of switches that can be set in the makefile. Here is a sample of the control panel at the top of the makefile:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
OPENMP   = yes
```

- Optimization enables the `-O3` optimization flag.
- Debugging enables the `-g` flag.
- Profiling enables the `-pg` flag. When profiling the code, you may want to significantly increase the number of lookups (with the `-l` flag) in order to wash out the initialization phase of the code.

## Running XSBench

To run XSBench with default settings, use the following command:
```bash
./XSBench -openmp-offload
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
Sets the simulation method, either "history" or "event". These options represent the history based or event based algorithms respectively. The default is the history based method.

- **-s [size]**
Sets the size of the Hoogenboom-Martin reactor model. There are four options: 'small', 'large', 'XL', and 'XXL'. By default, the 'large' option is selected.

- **-g [gridpoints]**
Sets the number of gridpoints per nuclide. By default, this value is set to 11,303.

- **-G [grid type]**
Sets the grid search type (unionized, nuclide, hash). Defaults to unionized.

- **-p [particles]**
Sets the number of particle histories to simulate. By default, this value is set to 500,000.

- **-l [lookups]**
Sets the number of cross-section (XS) lookups to perform per particle. By default, this value is set to 34.

- **-h [hash bins]**
Sets the number of hash bins (only relevant when using the hash lookup algorithm, as selected with "-G hash"). Default is 10,000.

- **-b [binary mode]**
This optional mode can read or write the simulation data structures to disk. Options are ("read" or "write").

- **-k [kernel]**
There are several optimized variants of the main kernel. All source bases run basically the same "baseline" kernel as default. Optimized kernels can be selected at runtime with this argument.

## Feature Discussion

### MPI Support

While XSBench is primarily used to investigate "on node parallelism" issues, some systems provide power & performance statistics batched in multi-node configurations. To accommodate this, XSBench provides an MPI mode which runs the code on all MPI ranks simultaneously.

### Verification Support

Legacy versions of XSBench had a special "Verification" compiler flag option to enable verification of the results. However, a much more performant and portable verification scheme was developed and is now used for all configurations -- therefore, it is not necessary to compile with or without the verification mode as it is always enabled by default.

### Binary File Support

Instead of initializing the randomized synthetic cross section data structures in XSBench every time it is run, you may optionally have XSBench generate a data set and write it to file. It can then be read on subsequent runs to speed up initialization.

## Algorithms

### Transport Simulation Styles

#### History-Based Transport

The default simulation model used in XSBench is the "history-based" model. In this model, parallelism is expressed over independent particle histories, with each particle being simulated in a serial fashion from birth to death:

```c
#pragma omp parallel for num_threads(omp_get_max_threads())
for (int i = 0; i < num_particles; i++) {
    // Independent
    while (particle_alive[i]) {
        MoveParticle(i);
        ProcessCollision(i);
    }
}
```

#### Event-Based Transport

An alternative simulation model is the "event-based" model. In this model, parallelism is instead expressed over different collision (or "event") types:

```c
#pragma omp parallel for num_threads(omp_get_max_threads())
for (int i = 0; i < num_events; i++) {
    // Dependent
    while (any_particles_alive) {
        // Independent
        for (int j = 0; j < num_particles; j++) {
            MoveParticle(j);
        }
        // Independent
        for (int k = 0; k < num_particles; k++) {
            ProcessCollision(k);
        }
        SortConsolidateParticles();
    }
}
```

## Optimized Kernels

If using the event-based model, we will be executing the lookup kernel in XSBench across all particles at once. While SIMD execution is possible using this method, typically issues can arise that greatly reduce SIMD efficiency.

```c
#pragma offload target(teams)
__attribute__((noinline)) void optimize_kernel(int num_particles) {
    #pragma omp parallel for num_threads(omp_get_max_threads())
    for (int i = 0; i < num_particles; i++) {
        // Material sort and energy sort
        sort_materials[i] = get_material(i);
        sort_energies[i] = get_energy(i);

        if (sort_materials[i] == 0 && sort_energies[i] == 0) {
            #pragma offload target(teams)
            macro_xs_kernel(sort_materials[i], sort_energies[i]);
        } else if (sort_materials[i] != 0 && sort_energies[i] == 0) {
            #pragma offload target(teams)
            fuel_macro_xs_kernel(sort_materials[i]);
        } else {
            // Other material kernels
        }
    }
}
```

## Citing XSBench

Papers citing the XSBench program in general should refer to:

> J. R. Tramm, A. R. Siegel, T. Islam, and M. Schulz, “XSBench - The Development and Verification of a Performance Abstraction for Monte Carlo Reactor Analysis,” presented at PHYSOR 2014 - The Role of Reactor Physics toward a Sustainable Future, Kyoto.

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