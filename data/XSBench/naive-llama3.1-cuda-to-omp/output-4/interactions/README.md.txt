Here's a possible translation of the README.md file to the OpenMP and offloading execution models:

```markdown
# XSBench: A Monte Carlo Reactor Analysis Mini-App
=============================================

## Introduction
---------------

XSBench is a mini-app representing a key computational kernel of the Monte Carlo neutron transport algorithm. Specifically, XSBench represents the continuous energy macroscopic neutron cross section lookup kernel.

## Compilation and Running
-------------------------

### Using Makefile

To compile XSBench with default settings, navigate to your selected source directory and use the following command:

```bash
make
```

You can alter compiler settings in the included `Makefile`.

### Debugging, Optimization & Profiling

There are also a number of switches that can be set in the makefile. Here is a sample of the control panel at the top of the makefile:

```make
OPTIMIZE = yes
DEBUG    = no
PROFILE  = no
MPI      = no
```

- Optimization enables the -O3 optimization flag.
- Debugging enables the -g flag.
- Profiling enables the -pg flag. When profiling the code, you may wish to significantly increase the number of lookups (with the -l flag) in order to wash out the initialization phase of the code.
- MPI enables MPI support in the code.

## Running XSBench
-----------------

To run XSBench with default settings, use the following command:

```bash
./XSBench
```

For non-default settings, XSBench supports the following command line options:

| Argument    | Description | Options     | Default |
|-------------|------------|---------------|------------|
|-m           | Simulation method | history, event | history|
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

## OpenMP and Offloading
-------------------------

XSBench has been parallelized using OpenMP for shared-memory architectures and offloaded to accelerators using OpenACC. The code uses OpenMP pragmas to specify parallel regions, while OpenACC directives are used to offload computations to accelerators.

For example, the following kernel is marked as offloadable:

```c
#pragma acc routine
void calculate_macro_xs(double p_energy, int mat, long n_isotopes,
                        long n_gridpoints, int * num_nucs,
                        double * concs, double * egrid, int * index_data,
                        NuclideGridPoint * nuclide_grids,
                        int * mats, double * macro_xs_vector, int grid_type, int hash_bins, int max_num_nucs)
```

The corresponding OpenACC directive is used to offload this kernel:

```c
#pragma acc routine
void calculate_macro_xs(double p_energy, int mat, long n_isotopes,
                        long n_gridpoints, int * num_nucs,
                        double * concs, double * egrid, int * index_data,
                        NuclideGridPoint * nuclide_grids,
                        int * mats, double * macro_xs_vector, int grid_type, int hash_bins, int max_num_nucs)
{
    // Offload kernel to accelerator
    #pragma acc kernels
    {
        for (int i = 0; i < n_isotopes * n_gridpoints; ++i) {
            // Perform macroscopic cross section lookup
            double xs_vector[5] = {0};
            calculate_micro_xs(p_energy, nuclide_grids[i].id, n_isotopes,
                               n_gridpoints, egrid, index_data, nuclide_grids,
                               i, xs_vector, grid_type, hash_bins);
            macro_xs_vector[i] += xs_vector[0] * concs[mats[i]];
        }
    }
}
```

The `#pragma acc kernels` directive is used to specify that the kernel should be offloaded to an accelerator. The corresponding OpenACC routine is marked with a `#pragma acc routine` directive.

Note that this is just one possible translation, and there may be other ways to parallelize and offload XSBench using OpenMP and OpenACC.
```