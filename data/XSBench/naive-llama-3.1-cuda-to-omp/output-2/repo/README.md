OpenMP-Offloading Execution Model
=====================================

Overview
--------

The OpenMP-Offloading execution model allows XSBench to offload compute-intensive tasks to accelerators (e.g., GPUs) using OpenMP directives. This enables XSBench to take advantage of the massively parallel processing capabilities of modern accelerators.

Key Features
-------------

*   **OpenMP Offloading**: XSBench uses OpenMP directives to offload computationally intensive tasks to accelerators.
*   **Accelerator Support**: Currently supports NVIDIA GPUs, but can be easily extended to other architectures.
*   **Kernel-Based Performance Model**: Utilizes a kernel-based performance model to accurately estimate execution times on the accelerator.

Optimization Techniques
-----------------------

### Key-Value Particle Sorting

To improve SIMD efficiency and cache performance, XSBench uses key-value particle sorting. This involves two sorts:

1.  **Material Sort**: First sort particles by material. This allows for adjacent particles in the vector to typically reside in the same type of material, reducing load imbalance.
2.  **Energy Sort**: Next, sort particles within each material by energy. This enables many or all adjacent particles to access the same energy indices in each nuclide and perform the same branching operations.

### OpenMP Offloading

After sorting, separate event kernels are called for each material in the simulation. These kernels offload computationally intensive tasks to accelerators using OpenMP directives.

Citing XSBench
---------------

Papers citing the XSBench program should refer to:

*   Tramm et al., "XSBench - The Development and Verification of a Performance Abstraction for Monte Carlo Reactor Analysis" (2014)

Bibtex Entry: