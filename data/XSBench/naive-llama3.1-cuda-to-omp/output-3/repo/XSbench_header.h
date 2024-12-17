#ifndef __XSBENCH_HEADER_H__
#define __XSBENCH_HEADER_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <cuda.h>
#include <omp.h>

// Grid types
#define UNIONIZED 0
#define NUCLIDE 1
#define HASH 2

// Simulation types
#define HISTORY_BASED 1
#define EVENT_BASED 2

// Binary Mode Type
#define NONE 0
#define READ 1
#define WRITE 2

#define STARTING_SEED 1070

#ifdef __cplusplus
typedef struct NuclideGridPoint {
    double energy;
    double total_xs;
    double elastic_xs;
    double absorbtion_xs;
    double fission_xs;
    double nu_fission_xs;
} NuclideGridPoint;
#endif

#pragma offload_attribute(push)
#pragma offload_attribute(align:64)

typedef struct SimulationData {
    int * num_nucs;                     // Length = length_num_nucs;
    double * concs;                     // Length = length_concs
    int * mats;                         // Length = length_mats
    double * unionized_energy_array;    // Length = length_unionized_energy_array
    int * index_grid;                   // Length = length_index_grid
    NuclideGridPoint * nuclide_grid;    // Length = length_nuclide_grid
    int length_num_nucs;
    int length_concs;
    int length_mats;
    int length_unionized_energy_array;
    long length_index_grid;
    int length_nuclide_grid;
    int max_num_nucs;
    unsigned long * verification;
    int length_verification;
} SimulationData;

#pragma offload_attribute(pop)

// Structures
typedef struct {
    double energy;
    double total_xs;
    double elastic_xs;
    double absorbtion_xs;
    double fission_xs;
    double nu_fission_xs;
} NuclideGridPoint;

#endif // __XSBENCH_HEADER_H__