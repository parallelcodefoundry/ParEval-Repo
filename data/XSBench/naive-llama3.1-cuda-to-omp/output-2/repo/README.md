// Table of Contents
// ==============
// 1. Compilation (OpenMP-Offload specific flags)
// 2. Running XSBench / Command Line Interface
// 3. Feature Discussion
//   - MPI Support
//     * No change necessary, still uses standard MPI library
//   - Verification Support
//     * Still uses same verification scheme as CUDA version
//   - Binary File Support
//     * Still reads/writes binary files as before (no offloading)
// 4. Algorithms
//   - Transport Simulation Styles
//     + History-Based Transport: still serial code, but can be executed on any thread or core
//     + Event-Based Transport: can now be parallelized using OpenMP threads
//   - Cross Section (XS) Lookup Methods
//     * Nuclide Grid: no change necessary
//     * Unionized Energy Grid: offloaded to GPU devices for execution if available
//     * Logarithmic Hash Grid: no change necessary
// 5. Optimized Kernels
//   - Still same optimized kernels as CUDA version, but now executed on host or GPU depending on availability

// Compilation (OpenMP-Offload specific flags)
// =======================================
// To compile XSBench with OpenMP and offloading support, use the following command:
// ```bash
// gcc -std=c99 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -o XSBench Main.cu io.cu Simulation.cu GridInit.cu XSutils.cu Materials.cu
// ```

// Running XSBench / Command Line Interface
// ======================================

// Since OpenMP is used, no command line flag for MPI support is necessary. However, users can still use the same CLI arguments as before.

// Feature Discussion

// MPI Support
// ===========

// No change necessary; still uses standard MPI library.

// Verification Support
// ==================

// Still uses same verification scheme as CUDA version.

// Binary File Support
// ===================

// Still reads/writes binary files as before (no offloading).

// Algorithms

// Transport Simulation Styles

// History-Based Transport: still serial code, but can be executed on any thread or core.
// ```c
// #pragma omp parallel for num_threads(16)
// for each particle do
//  while particle is alive do
//     Move particle to collision site
//     Process particle collision
// ```
// Event-Based Transport: can now be parallelized using OpenMP threads.
// ```c
// #pragma omp parallel for num_threads(16)
// Get vector of source particles
// while any particles are alive do
//  for each living particle do
//      Move particle to collision site
//  for each living particle do
//      Process particle collision
// ```

// Cross Section (XS) Lookup Methods

// Nuclide Grid: no change necessary.

// Unionized Energy Grid: offloaded to GPU devices for execution if available.
// ```c
// #pragma omp target data map(tofrom:x : GSD.num_nucs[:length_num_nucs]) 
// #pragma omp target update self(GSD.num_nucs[:length_num_nucs])
// macroscopic XS = 0
// for each nuclide in M do:
//     index = grid_search( n_gridpoints, E, &GSD.nuclide_grids[nuc*n_gridpoints] );
//     interpolate data from grid[nuclide, index]
//     macroscopic XS += data
// ```

// Logarithmic Hash Grid: no change necessary.

// Optimized Kernels

// Still same optimized kernels as CUDA version, but now executed on host or GPU depending on availability.