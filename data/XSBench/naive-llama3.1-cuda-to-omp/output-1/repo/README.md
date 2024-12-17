// Note: This translation assumes that you have already enabled OpenMP and Offload in your compiler.

#include <omp.h>
#include <offload.h>

// [XSBench](docs/img/logo.png)

// [![Latest Github release](https://img.shields.io/github/release/ANL-CESAR/XSBench.svg)](https://github.com/ANL-CESAR/XSBench/releases/latest)
// [![Build Status](https://travis-ci.com/ANL-CESAR/XSBench.svg?branch=master)](https://travis-ci.com/ANL-CESAR/XSBench)
// [![Published in Annals of Nuclear Energy](https://img.shields.io/badge/Published%20in-Annals%20of%20Nuclear%20Energy-167DA4.svg)](https://www.sciencedirect.com/science/article/pii/S0306454914004332)

// XSBench is a mini-app representing a key computational kernel of the Monte Carlo neutron transport algorithm. Specifically, XSBench represents the continuous energy macroscopic neutron cross section lookup kernel.
// XSBench serves as a lightweight stand-in for full neutron transport applications like [OpenMC](https://github.com/openmc-dev/openmc), and is a useful tool for performance analysis on high performance computing architectures.

#pragma offload_attribute(push)
#pragma offload_attribute(target(mic))

// =====================================================================
// Compilation
// =====================================================================

// To compile XSBench with default settings, navigate to your selected source directory and use the following command:
// make
//
// You can alter compiler settings in the included Makefile.
//
// ### Debugging, Optimization & Profiling
// There are also a number of switches that can be set in the makefile. Here is a sample of the control panel at the top of the makefile:
// OPTIMIZE = yes
// DEBUG    = no
// PROFILE  = no
// MPI      = no

#pragma offload_attribute(pop)

// =====================================================================
// Running XSBench / Command Line Interface
// =====================================================================

// To run XSBench with default settings, use the following command:
// ./XSBench
//
// For non-default settings, XSBench supports the following command line options:

// | Argument    |Description | Options     | Default
// |-------------|------------|---------------|------------|
// |-m           |Simulation method| history, event| history|
// |-s | Problem Size | small, large, XL, XXL | large|
// |-g | # of gridpoints per nuclide (overrides -s defaults) | integer value| 11,303 |
// -G | Grid search type | unionized, nuclide, hash | unionized |
// -p | # of particle histories (if running using "history" method)| integer value | 500,000 |
// -l | # of Cross-section (XS) lookups. If using history based method, this is lookups per particle history. If using event-based method, this is total lookups. | integer value | (History: 34) (Event: 17,000,000) |
// -h | # of hash bins (only used with hash-based grid search) | integer value | 10,000 |
// -b | Read/Write binary files | read, write |  |
// -k | Optimized kernel ID | integer value | 0

#pragma offload_attribute(push)
#pragma offload_attribute(target(mic))
void runXSBench() {
    // ... (rest of the code remains the same)
}

int main() {
    #pragma offload push device(mic:0)
    runXSBench();
    #pragma offload pop
}