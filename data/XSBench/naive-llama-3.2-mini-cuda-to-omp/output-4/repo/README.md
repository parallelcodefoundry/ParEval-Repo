# XSBench: Monte Carlo Reactor Analysis

## Overview
XSBench is a high-performance Python library for Monte Carlo reactor analysis. It provides a flexible and customizable framework for simulating various reactor systems.

## Performance Abstraction
XSBench abstracts away the performance details of underlying hardware, allowing users to easily switch between different execution models (e.g., OpenMP offload).

## Usage
1. Compile XSBench with OpenMP support using `gcc -fopenmp xsbench.c -o xsbench`.
2. Run `python` to execute a simulation.

## Citing XSBench
Papers citing the XSBench program in general should refer to:

[J. R. Tramm, A. R. Siegel, T. Islam, and M. Schulz, ���XSBench - The Development and Verification of a Performance Abstraction for Monte Carlo Reactor Analysis,��� presented at PHYSOR 2014 - The Role of Reactor Physics toward a Sustainable Future, Kyoto. https://www.mcs.anl.gov/papers/P5064-0114.pdf](https://www.mcs.anl.gov/papers/P5064-0114.pdf)

Bibtex Entry:
