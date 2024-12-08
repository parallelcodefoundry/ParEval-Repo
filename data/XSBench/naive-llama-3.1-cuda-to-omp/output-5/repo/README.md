# XSBench: A Performance Abstraction for Monte Carlo Reactor Analysis

XSBench is a software package designed to measure and improve the performance of Monte Carlo reactor simulations. It provides a flexible and extensible framework for simulating various types of reactors, including pressurized water reactors (PWRs), boiling water reactors (BWRs), and small modular reactors (SMRs).

## Key Features

*   **OpenMP Offloading**: XSBench supports OpenMP offloading, which allows users to execute parallel regions on a GPU or other accelerator devices.
*   **Unionized Grid**: The Unionized Grid is the default grid type used in XSBench. It provides efficient memory access and minimizes memory usage by storing only unique data points.
*   **Nuclide Grid**: In addition to the Unionized Grid, XSBench also supports the Nuclide Grid, which stores all nuclide data for each particle in a separate array.
*   **Hash-based Access**: XSBench uses hash-based access to improve performance when accessing large datasets.
*   **Support for Various Simulation Methods**: XSBench supports multiple simulation methods, including Monte Carlo, Discrete-Ordinates, and Deterministic Transport.

## Citing XSBench

Papers citing the XSBench program in general should refer to:

>J. R. Tramm, A. R. Siegel, T. Islam, and M. Schulz, ���XSBench - The Development and Verification of a Performance Abstraction for Monte Carlo Reactor Analysis,��� presented at PHYSOR 2014 - The Role of Reactor Physics toward a Sustainable Future, Kyoto. https://www.mcs.anl.gov/papers/P5064-0114.pdf

Bibtex Entry:
